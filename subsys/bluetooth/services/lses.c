/** @file
 *  @brief LSES Service sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <init.h>

#include <drivers/lora.h>

#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/lses.h>

#define MAX_DATA_LEN 20

#define LOG_LEVEL CONFIG_BT_LSES_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lses);

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

static uint8_t lses_blsc;

static struct lora_modem_config config;
int frequencies[8] =  {869500000 ,868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000};

/*static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connnnnnnnnwwected\n");
	}
}

// triggered when disconnected from phone
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}
*/

// when descriptor changed at phone (for enableing notifications)
static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LSES notifications %s", notif_enabled ? "enabled" : "disabled");
}

void change_config(uint8_t* pu, bool tx) {
	if(connected) {
		printk("connection visible in lses!!!\n");
	}
	const struct device *lora_dev;

	config.frequency = frequencies[*pu];
	printk("fr data %d\n", *pu);
	pu++;

	config.bandwidth = *pu;
	printk("bw data %d\n", *pu);
	pu++;

	config.datarate = *pu + 7;
	printk("sf data %d\n", *pu);
	pu++;

	config.preamble_len = 8;

	config.coding_rate = *pu + 1;
	printk("cr data %d\n", *pu);
	pu++;

	config.tx_power = 18 - (*pu * 2);
	printk("pw data %d\n", *pu);

	config.tx = tx;

	lora_dev = device_get_binding(DEFAULT_RADIO);

	int ret;
	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}
}

// implemented at bottom of file (declared here for use in nexxt function)
int bt_lses_notify(int8_t type_of_notification);


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////			callbacks and thread definitions for Explore mode			/////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////


// gets 5 bytes from phone indicating LoRa configuration settings (callback for the corresponding characteristic)
static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	uint8_t *pu = (uint8_t *) buf;
	change_config(pu, true);

	bt_lses_notify(-2);
	return 0;
}



#define STACK_SIZE2 2048
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area2, STACK_SIZE2);

struct k_thread thread_data2;

uint8_t ping_content[MAX_DATA_LEN];
uint16_t ping_len;

// function for waiting for ping return on separate thread
void wait_for_ping_return(void *a, void *b, void *c) {
	int16_t rssi;
	int8_t snr;
	int l = -1;

	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);				

	config.tx = false;
	lora_config(lora_dev, &config);						
		
	char resp[MAX_DATA_LEN] = {0};
	l = lora_recv(lora_dev, resp, MAX_DATA_LEN, K_SECONDS(3),	
				&rssi, &snr);

	if (l < 0) {
		LOG_ERR("no response received");	
	} else {
		if(memcmp(ping_content, resp, ping_len * sizeof(uint8_t))) {
			printk("ping is okay\n");
			bt_lses_notify(-3);									// check if received ping exactly matches sent ping
		} else {
			printk("ping does not match\n");	
			bt_lses_notify(-4);	
		}
	}

	config.tx = true;
	lora_config(lora_dev, &config);	
}




// gets a byte array containing the msg a LoRa transmission is supposed to contain for explore mode
static ssize_t send_command_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	// extract LoRa message
	char data[len];
	char *pc = (char *) buf;
	bool isPing = false;
	if(*pc == '&') {
		isPing = true;
	}
	for(uint16_t i = 0; i < len; i++) {
		data[i] = *pc;
		pc++;
	}
	data[len] = '.';
	printk("msg: %s\n", data);

	const struct device *lora_dev;
	int ret;

	// send message
	lora_dev = device_get_binding(DEFAULT_RADIO);

	ret = lora_send(lora_dev, data, MAX_DATA_LEN);

	if(isPing) {
		ping_len = len;
		for(int16_t i = 0; i < len; i++) {
			ping_content[i] = *pc;
			pc++;
		}

		k_thread_create(&thread_data2, stack_area2,
				K_THREAD_STACK_SIZEOF(stack_area2),
				wait_for_ping_return,
				NULL, NULL, NULL,
				TT_PRIORITY, 0, K_NO_WAIT);
	}
	
	return 0;
}


#define STACK_SIZE 16384
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area0, STACK_SIZE);

struct k_thread thread_data0;
k_tid_t thread0_tid;

uint8_t loop_data[255];
uint16_t loop_data_length = 0;

// to control the sending LoRa messages loop in explore mode
static uint16_t number_of_messages = 0;
static uint8_t time_between_msgs = 0;

void exec_loop(void *a, void *b, void *c) {
	uint16_t len = loop_data_length;
	char data[len];
	
		for(uint16_t i = 0; i < len; i++) {
			data[i] = loop_data[i];
		}
		data[len] = '.';

		const struct device *lora_dev;

		// send message
		lora_dev = device_get_binding(DEFAULT_RADIO);

		uint16_t i = 0;
		while (i < number_of_messages) {
			lora_send(lora_dev, data, MAX_DATA_LEN);
			
			LOG_INF("Data sent!");
			k_sleep(K_SECONDS(time_between_msgs));
			i++;
		}

	return;
}

// prepare or start a LoRa message sending loop ()
static ssize_t loop_command_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{	
	printk("at loop c callback...\n");

	char *pc = (char *) buf;
	if(*pc == '!') {						// set number of loops in explore mode
		pc++;
		uint16_t i = atoi(pc);
		number_of_messages = i;						
	} else if (*pc == '#') {				// set the time between messages
		pc++;				
		uint16_t i = atoi(pc);
		time_between_msgs = i;
	} else if(*pc == '$') {
		if(thread0_tid != NULL) {
			k_thread_abort(thread0_tid);
			printk("loop thread canceled\n");
		}
	} else {								// start the loop
		loop_data_length = len;
		for(uint16_t i = 0; i < len; i++) {
			loop_data[i] = *pc;
			pc++;
		}

		loop_data_length = len;

		thread0_tid = k_thread_create(&thread_data0, stack_area0,
			K_THREAD_STACK_SIZEOF(stack_area0),
			exec_loop,
			NULL, NULL, NULL,
			TT_PRIORITY, 0, K_NO_WAIT);
	}

	return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////			callback and thread definitions for Experiment mode			/////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area1, STACK_SIZE);

struct k_thread thread_data1;
k_tid_t thread1_tid;

uint8_t random_d [200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,
					37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,
					73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,
					107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,
					134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
					161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
					188,189,190,191,192,193,194,195,196,197,198,199};

// experiment send thread code
void exec_experiment(void *a, void *b, void *c) {
	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);

	// wait for experiment started notification or a ping
	int16_t rssi;
	int8_t snr;
	int l = 9;;
	uint8_t data[MAX_DATA_LEN] = {0};
	bool experiment_started = false;

	while(!experiment_started) {
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_FOREVER,
					&rssi, &snr);
		
		if(data[0] == 33) {					// in this case it is a ping instead of the experiment settings, which start the experiment procedure
			config.tx = true;
			lora_config(lora_dev, &config);	
			lora_send(lora_dev, data, 5);
			data[0] = -1;
			config.tx = false;
			lora_config(lora_dev, &config);	
			printk("responded to ping\n");			
		} else {
			experiment_started = true;
		}
	}
	

	// reconfigure device for sending and send received data as ACK, listen for retransmission until delay counted down
	bool experiment_ready = false;
	while(!experiment_ready) {
		int ret;


		char delay[l-9];
		for(int16_t i = 0; i < l; i++) {
			if(i > 8) {														// get experiment start delay
				delay[i-9] = data[i];
			}
		}
		uint16_t d = atoi(delay); 


		config.tx = true;
		ret = lora_config(lora_dev, &config);
		
		k_sleep(K_MSEC(200));
		ret = lora_send(lora_dev, data, MAX_DATA_LEN);						// send received experiment settings back as ACK

		config.tx = false;
		ret = lora_config(lora_dev, &config);
		
		printk("delay countdown (re-)started\n");
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(d),			// listen for retransmission in case ACK was lost as long as the specified delay
				&rssi, &snr);
		if (l < 0) {
			experiment_ready = true;
			printk("experiment started\n");
		} 
	}



	// start experiment
	char transmission_data[data[2]]; 	// data[2] contains msg length
	transmission_data[5] = '_';

	config.tx = true;
	int ret;
	for(uint8_t i = 0; i < 8; i++) {
		if(((data[4] >> i)  & 0x01) == 1) {					// the 4th byte of the settings byte array represents the frequencies to use
			config.frequency = frequencies[i];				// if a bit in that byte is set, the corresponding frequency will be used;
			transmission_data[0] = (char) i + 48;
			printk("frequency: %d\n", frequencies[i]);
		} else {
			continue;
		}

		for(uint8_t j = 0; j < 3; j++) {
			if(((data[5] >> j)  & 0x01) == 1) {
				config.bandwidth =  j;
				transmission_data[1] = (char) j + 48;
				printk("bandwidth: %d\n", j);
			} else {
				continue;
			}
			for(uint8_t k = 0; k < 6; k++) {
				if(((data[6] >> k)  & 0x01) == 1) {
					config.datarate =  k + 7;
					transmission_data[2] = (char) k + 48;
					printk("data rate: %d\n", k+7);
				} else {
					continue;
				}
				for(uint8_t l = 0; l < 4; l++) {
					if(((data[7] >> l)  & 0x01) == 1) {
						config.coding_rate =  l + 1;
						transmission_data[3] = (char) l + 48;
						printk("coding rate: %d\n", l+1);
					} else {
						continue;
					}
					for(uint8_t m = 0; m < 8; m++) {
						if(((data[8] >> m)  & 0x01) == 1) {
							config.tx_power =  18 - (m * 2);
							transmission_data[4] = (char) m + 48;
							
							ret = lora_config(lora_dev, &config);
							
							for(uint8_t n = 0; n < data[0]; n++) {											// data[0] contains the number of LoRa transmissions per parameter combination
								transmission_data[6] = (char) n / 100 + 48;									// include numbering into transmission content (as String (3 bytes) not as byte (1 byte))
								transmission_data[7] = (char) n / 10 + 48;									// this line needs to be changed if max number of transmissions per combination is changed (it is 100 now)
								transmission_data[8] = (char) n % 10 + 48;

								for(uint8_t p = 9; p < (data[2] - 1); p++) {								// data[2] contains message length (length of the transmitted content)
									transmission_data[p] = random_d[(n * (data[2] - 9) + (p - 9)) % 200];	// fills the message up with with random payload data until desired message length
								}														
								transmission_data[data[2] - 1] = '.';

								printk("transmission data: %s\n", transmission_data);
								ret = lora_send(lora_dev, transmission_data, data[2]);
								
								k_sleep(K_MSEC(data[1] * 1000));											// data[1] contains the number of seconds between transmissions
							}

							k_sleep(K_MSEC(5000));															// wait 5 seconds between combinations
						} else {
							continue;
						}
					}
				}
			}
		}
	}

	printk("end of experiment...........\n");

	return;
}



// prepare sender for experiment and start sending thread or cancel prepared sender
static ssize_t prepare_sender_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);

	if(len == 1) {									// an already prepared sender is supposed to be canceled
		config.tx = true;
		lora_config(lora_dev, &config);
		if(thread1_tid != NULL) {
			k_thread_abort(thread1_tid);
			printk("thread canceled\n");
		}
		return 0;
	}

	char *pc = (char *) buf;

	// configure and prepare experiment mode
	change_config(pc, false);
	bt_lses_notify(-2);

	thread1_tid = k_thread_create(&thread_data1, stack_area1,
		K_THREAD_STACK_SIZEOF(stack_area1),
		exec_experiment,
		NULL, NULL, NULL,
		TT_PRIORITY, 0, K_NO_WAIT);

	printk("sender ready\n");

	return 0;
}

// Lora Eval Service Declaration: service, descriptor, 1 notification characteristic, 3 write characteristics
BT_GATT_SERVICE_DEFINE(lses_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_LSES),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_STAT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(lec_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_CHANGE_CONFIG, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, change_config_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_SEND_COMMAND, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, send_command_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_LOOP_COMMAND, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, loop_command_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_EXP_PREP, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, prepare_sender_cb, NULL),
);

static int lses_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	lses_blsc = 0x01;

	return 0;
}

// notify phone about anything (currently only distinguishable in type of message (and only used for -2 = config changed))
int bt_lses_notify(int8_t type_of_notification)
{
	int rc;
	static int8_t notifier[1];
	notifier[0] = type_of_notification;

	rc = bt_gatt_notify(NULL, &lses_svc.attrs[1], &notifier, sizeof(notifier));
	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(lses_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
