/** @file
 *  @brief LRES Service sample
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

#define MAX_DATA_LEN 20
#define MAX_TRANSM_LEN 255

#define LOG_LEVEL CONFIG_BT_LRES_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lres);

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

static uint8_t lres_blsc;
static struct lora_modem_config config;
int frequencies[8] =  {869500000 ,868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000};

// when descriptor changed at phone (for enableing notifications)
static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LRES notifications %s", notif_enabled ? "enabled" : "disabled");
}

// implemented at bottom of file (declared here for use in next function)
int bt_lres_notify(const void *data, uint8_t type_of_notification);

// for convenience: change LoRa parameter configuration according to arguments
void change_config(uint8_t* pu, bool tx) {
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

	config.tx_power = *pu + 5;
	printk("pw data %d\n", *pu);

	config.tx = tx;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	int ret;
	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////			callback and thread definition for Explore mode			/////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define STACK_SIZE 16384
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area0, STACK_SIZE);

struct k_thread thread_data0;
k_tid_t thread0_tid;

// separate LoRa receive thread code
void receive_lora(void *a, void *b, void *c) {
	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);

	int len;
	uint8_t data[MAX_DATA_LEN] = {0};
	
	int16_t rssi;
	int8_t snr;

	while (1) {
		len = lora_recv(lora_dev, data, MAX_DATA_LEN, K_FOREVER,
				&rssi, &snr);
		
		uint8_t ndata[2] = {0};
		rssi = (uint8_t) -rssi; // negated to fit into an unsigned int (original value is negative)
		ndata[0] = rssi;
		ndata[1] = snr;

		// notfiy phone with sent LoRa message and other data
		bt_lres_notify(ndata, 0);
		bt_lres_notify(data, 1);
		
		LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
			log_strdup(data), rssi, snr);
	}
}

// gets 5 bytes from phone indicating LoRa configuration settings (callback for the corresponding characteristic) and starts receiving thread
static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	if(thread0_tid != NULL) {
		k_thread_abort(thread0_tid);
	}

	uint8_t *pu = (uint8_t *) buf;		
	change_config(pu, false);
	printk("frequency value after change config: %d\n", config.frequency);

	int8_t bt_data[1] = {-2};
	bt_lres_notify(bt_data, 2);

	// (re-) start the LoRa receiving thread after changing config
	thread0_tid = k_thread_create(&thread_data0, stack_area0,
			K_THREAD_STACK_SIZEOF(stack_area0),
			receive_lora,
			NULL, NULL, NULL,
			TT_PRIORITY, 0, K_NO_WAIT);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////			callback and thread definitions for Experiment mode			/////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////
////// for experiment ///
/////////////////////////
uint8_t random_d [200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,
					37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,
					73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,
					107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,
					134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
					161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
					188,189,190,191,192,193,194,195,196,197,198,199};


K_THREAD_STACK_DEFINE(stack_area1, STACK_SIZE);

struct k_thread thread_data1;

uint8_t experiment_data[18];
uint16_t exp_data_length = 0;

// experiment receive thread code
void exec_experiment(void *a, void *b, void *c) {
	// initialize stuff
	uint8_t exp_data[exp_data_length];
	uint16_t len = exp_data_length;

	for(int16_t i = 0; i < len; i++) {
		exp_data[i] = experiment_data[i];	
	}

	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);
	
	int ret;
	int16_t rssi;
	int8_t snr;
	int l = -1;
	uint8_t data[MAX_DATA_LEN] = {0};					// data array for ACK from sender device
	bool exp_started = false;



	// sends experiment settings and waits for 2 seconds in a loop until ACK arrives
	while(!exp_started) {
		ret = lora_send(lora_dev, exp_data, len);					// send experiment settings to other board

		config.tx = false;
		ret = lora_config(lora_dev, &config);						// configure as receiver, wait 2 seconds for ACK
			
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(2),	// wait for ACK
					&rssi, &snr);
		if (l < 0) {
			LOG_ERR("no ACK received");	
			int8_t bt_data[1] = {-5};
			bt_lres_notify(bt_data, 2);	
		} else {
			if(memcmp(exp_data, data, len * sizeof(uint8_t)) == 0) {
				printk("ACK is okay\n");
				exp_started = true;									// check if received data exactly matches sent data
			}
		}
	}



	// get experiment start delay
	char delay[5];													
	for(int16_t i = 0; i < l; i++) {
		if(i > 8) {			
			delay[i-9] = exp_data[i];
		}
	}
	uint16_t d = atoi(delay); 


	
	// start experiment as receiver
	bool first_iteration = true;								// first iteration has the delay added to its lora receive timeout							
	uint8_t transmission_data[MAX_TRANSM_LEN] ={0};				// exp_data[2] contains msg length
	uint8_t compare_data[MAX_TRANSM_LEN] ={0};
	config.tx = false;

	compare_data[5] = '_';

	for(uint8_t i = 0; i < 8; i++) {
		if(((exp_data[4] >> i)  & 0x01) == 1) {					// the 4th byte of the settings byte array represents the frequencies to use
			config.frequency = frequencies[i];					// if a bit in that byte is set, the corresponding frequency will be used;
			compare_data[0] = (char) i + 48;
			printk("frequency: %d\n", frequencies[i]);
		} else {
			continue;
		}

		for(uint8_t j = 0; j < 3; j++) {
			if(((exp_data[5] >> j)  & 0x01) == 1) {
				config.bandwidth =  j;
				compare_data[1] = (char) j + 48;
				printk("bandwidth: %d\n", j);
			} else {
				continue;
			}
			for(uint8_t k = 0; k < 6; k++) {
				if(((exp_data[6] >> k)  & 0x01) == 1) {
					config.datarate =  k + 7;
					compare_data[2] = (char) k + 48;
					printk("data rate: %d\n", k+7);
				} else {
					continue;
				}
				for(uint8_t m = 0; m < 4; m++) {
					if(((exp_data[7] >> m)  & 0x01) == 1) {
						config.coding_rate =  m + 1;
						compare_data[3] = (char) m + 48;
						printk("coding rate: %d\n", m+1);
					} else {
						continue;
					}
					for(uint8_t p = 0; p < 8; p++) {
						if(((exp_data[8] >> p)  & 0x01) == 1) {
							config.tx_power =  p + 5;
							compare_data[4] = (char) p + 48;
							printk("power: %d\n", p+1);
							ret = lora_config(lora_dev, &config);
							
							int64_t time_stamp;
							int64_t milliseconds_spent = 0;
							time_stamp = k_uptime_get();
							int64_t iteration_time = exp_data[0] * exp_data[1] * 1000;					// exp_data[0] * exp_data[1] = # LoRa transmissions * time between transmissions
							
							if(first_iteration) {
								iteration_time += 1000 * d;												// count down delay as part of the LoRa receive timing
								first_iteration = false;
							} else {
								iteration_time += 5000;													// delay between iterations as part of the LoRa receive timing
							}

							uint8_t last_data_8 = 0;
							while(iteration_time > 0) {													// exp_data[0] contains the number of LoRa transmissions per parameter combination
								l = lora_recv(lora_dev, transmission_data, MAX_TRANSM_LEN, K_MSEC(iteration_time),
										&rssi, &snr);
								
								if(last_data_8 != transmission_data[8]) {								// checking if lora_recv just timed out or if something was actually received
									LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
										log_strdup(transmission_data), rssi, snr);


									char tmp[3];
									for (int i = 0; i < 3; i++) {
										compare_data[i + 6] = transmission_data[i + 6];					// write msg number to compare data only after receive, because no way to know how many msgs were lost before this one
										tmp[i] = transmission_data[i + 6];								// use the msg number (like 008) to determine which random data the transmission should contain
									}
									int msg_num = atoi(tmp);	


									for(uint8_t p = 9; p < (exp_data[2] - 1); p++) {					// exp_data[2] contains message length (length of the transmitted content)
										compare_data[p] = random_d[(msg_num * (exp_data[2] - 9) + (p - 9)) % 200];			
									}																	// fills the message up with with random payload data until desired message length
									compare_data[exp_data[2] - 1] = '.';

									bool same_content = true;
									for(int z = 0; z < exp_data[2]; z++) {
										if(compare_data[z] != transmission_data[z]) {
											same_content = false;
											break;
										}
									}


									uint8_t ndata[2] = {0};
									rssi = (uint8_t) -rssi; 											// negated to fit into an unsigned int (original value is negative)
									ndata[0] = rssi;
									ndata[1] = snr;

									bt_lres_notify(ndata, 0);
									if(same_content) {
										transmission_data[9] = 't';
									} else {
										transmission_data[9] = 'f';
									}
									transmission_data[10] = '.';										
									bt_lres_notify(transmission_data, 1);								// send results to phone to monitor experiment
									last_data_8 = transmission_data[8];
								}

								milliseconds_spent = k_uptime_delta(&time_stamp);
								time_stamp = k_uptime_get();	
								iteration_time = iteration_time - milliseconds_spent;						
							}

							printk("end of iteration\n");
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


/////////////////////////
////// for pings ////////
/////////////////////////

#define STACK_SIZE2 2048
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area2, STACK_SIZE2);

struct k_thread thread_data2;

// function for waiting for ping return on separate thread
void wait_for_ping_return(void *a, void *b, void *c) {
	int16_t rssi;
		int8_t snr;
		int l = -1;
		char ping[5] = {'!', 'p', 'i', 'n', 'g'};

		const struct device *lora_dev;
		lora_dev = device_get_binding(DEFAULT_RADIO);
		lora_send(lora_dev, ping, 5);						

		config.tx = false;
		lora_config(lora_dev, &config);						
			
		char resp[5] = {0};
		l = lora_recv(lora_dev, resp, MAX_DATA_LEN, K_SECONDS(3),	
					&rssi, &snr);

		if (l < 0) {
			LOG_ERR("no response received");	
		} else {
			if(memcmp(ping, resp, 5 * sizeof(uint8_t)) == 0) {
				printk("ping is okay\n");
				int8_t bt_data[1] = {-3};
				bt_lres_notify(bt_data, 2);									// check if received ping exactly matches sent ping
			} else {
				printk("ping does not match\n");	
				int8_t bt_data[1] = {-4};
				bt_lres_notify(bt_data, 2);	
			}
		}

		config.tx = true;
		lora_config(lora_dev, &config);	
}


// receives and changes the initial lora config for pre experiment lora communication OR
// receives the experiment settings via BLE and starts the receiving side of the experiment on a new thread OR
// receives a ping command and pings the sender device
static ssize_t experiment_settings_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	if(thread0_tid != NULL) {
		k_thread_abort(thread0_tid);			// if still receiving on an explore mode thread, cancel this thread
	}

	uint8_t *pu = (uint8_t *) buf;
	if(len == 5) {								// receive and change config (on connection)
		change_config(pu, true);
		int8_t bt_data[1] = {-2};
		bt_lres_notify(bt_data, 2);
		return 0;
	}

	if(len == 1) {								// it is a ping
		k_thread_create(&thread_data2, stack_area2,
			K_THREAD_STACK_SIZEOF(stack_area2),
			wait_for_ping_return,
			NULL, NULL, NULL,
			TT_PRIORITY, 0, K_NO_WAIT);
		return 0;	
	}

	exp_data_length = len;						// start experiment
	for(int16_t i = 0; i < len; i++) {
		experiment_data[i] = *pu;
		pu++;
	}

	k_thread_create(&thread_data1, stack_area1,
			K_THREAD_STACK_SIZEOF(stack_area1),
			exec_experiment,
			NULL, NULL, NULL,
			TT_PRIORITY, 0, K_NO_WAIT);

	return 0;
}

// Lora Eval Service Declaration: service, descriptor, 1 notification characteristic, 2 write characteristics
BT_GATT_SERVICE_DEFINE(lres_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_LRES),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_STAT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(lec_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_CHANGE_CONFIG, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, change_config_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_EXP, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, experiment_settings_cb, NULL),
);

static int lres_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	lres_blsc = 0x01;

	return 0;
}

// notify phone about anything
int bt_lres_notify(const void *data, uint8_t type_of_notification)
{
	int rc;

	if(type_of_notification == 0) {		// RSSI/SNR notification
		uint8_t *pu = (uint8_t *) data;

		static uint8_t stats[3];

		// put -1 at index 0 to distinguish from msg notification
		stats[0] = -1;
		stats[1] = *pu;
		pu++;
		stats[2] = *pu;

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &stats, sizeof(stats));
	} else if(type_of_notification == 1){							// msg notification
		char *pc = (char *) data;
		char data[MAX_DATA_LEN];
	
		for(uint16_t i = 0; i < MAX_DATA_LEN; i++) {
			data[i] = *pc;
			pc++;
		}

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &data, sizeof(data));
	} else {		// notify about config changed
		static int8_t notifier[1];
		char *pc = (char *) data;
		notifier[0] = *pc;

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &notifier, sizeof(notifier));
	}
	

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(lres_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);