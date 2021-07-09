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

bool bt_lres_connected;

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

	config.tx_power = 18 - (*pu * 2);
	printk("pw data %d\n", *pu);

	config.tx = tx;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}
	//lora_init(lora_dev);

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
		
		if(data[0] == '&') {		// this was a ping request
			config.tx = true;
			lora_config(lora_dev, &config);	
			lora_send(lora_dev, data, len);
			config.tx = false;
			lora_config(lora_dev, &config);	
			continue;
		}
		
		uint8_t ndata[3] = {0};
		rssi = (uint8_t) -rssi; // negated to fit into an unsigned int (original value is negative)
		ndata[0] = rssi;
		ndata[1] = snr;
		ndata[2] = -1; 			// array index for bit errors in experiments (probably unnecessary to assign)

		// notfiy phone with sent LoRa message and other data
		bt_lres_notify(ndata, 0);
		bt_lres_notify(data, 1);
		
		LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
			log_strdup(data), rssi, snr);
	}
}

bool reconnect = false;
// gets 5 bytes from phone indicating LoRa configuration settings (callback for the corresponding characteristic) and starts receiving thread
static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	if(thread0_tid != NULL) {
		k_thread_abort(thread0_tid);
	}

	uint8_t *pu = (uint8_t *) buf;		
	change_config(pu, false);

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
uint8_t random_d [200] = {96, -76, 32, -69, 56, 81, -39, -44, 122, -53, -109, 61, -66, 112, 57, -101,
				-10, -55, 45, -93, 58, -16, 29, 79, -73, 112, -23, -116, 3, 37, -12, 29, 62,
				-70, -8, -104, 109, -89, 18, -56, 43, -51, 77, 85, 75, -16, -75, 64, 35, -62,
				-101, 98, 77, -23, -17, -100, 47, -109, 30, -4, 88, 15, -102, -5, 8, 27, 18, 
				-31, 7, -79, -24, 5, -14, -76, -11, -16, -15, -48, 12, 45, 15, 98, 99, 70, 112, 
				-110, 28, 80, 88, 103, -1, 32, -10, -88, 51, 94, -104, -81, -121, 37, 56, 85, 
				-122, -76, 31, -17, -14, 5, -76, -32, 90, 0, 8, 35, -9, -117, 95, -113, 92, 2, 
				67, -100, -24, -10, 122, 120, 29, -112, -53, -26, -65, 26, -25, -14, -68, 64, 
				-92, -105, 9, -96, 108, 14, 49, 73, -101, -16, 41, 105, -54, 66, -46, 3, -27, 
				102, -68, -58, -106, -34, 8, -6, 1, 2, -96, -3, 46, 35, 48, -80, -106, 74, -69, 
				124, 68, 48, 32, -34, 28, -83, 9, -65, -42, 56, 31, -5, -108, -38, -81, -69, 
				-112, -60, -19, -111, -96, 97, 58, -47, -36, 75, 71, 3};

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
			LOG_ERR("no ACK received.");	
			int8_t bt_data[1] = {-5};
			bt_lres_notify(bt_data, 2);	
		} else {
			if(memcmp(exp_data, data, len * sizeof(uint8_t)) == 0) {
				printk("ACK is okay\n");
				int8_t bt_data[1] = {-6};
				bt_lres_notify(bt_data, 2);	
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

	if(config.tx_power > 10) {									// wait for reconnection after disconnect when sending with over 10 dbm
		d -= 14;
		k_sleep(K_MSEC(14000));
		int8_t bt_data[1] = {-6};
		bt_lres_notify(bt_data, 2);
	}


	
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
							config.tx_power =  18 - (p * 2);
							compare_data[4] = (char) p + 48;
							printk("power: %d\n", p+1);
							ret = lora_config(lora_dev, &config);
							
							int64_t time_stamp;
							int64_t milliseconds_spent = 0;
							time_stamp = k_uptime_get();
							int64_t iteration_time = exp_data[0] * exp_data[1] * 1000;					// exp_data[0] * exp_data[1] = # LoRa transmissions * time between transmissions
							
							if(first_iteration) {
								iteration_time += (1000 * d);												// count down delay as part of the LoRa receive timing
								first_iteration = false;
							}
							iteration_time += 5000 ;													// delay between iterations as part of the LoRa receive timing
							printk("iteration time calc: %lld\n", iteration_time);														// time to compensate for time on air in first iteration

							uint8_t last_data_8 = 0;
							while(iteration_time > 0) {													// exp_data[0] contains the number of LoRa transmissions per parameter combination
								l = lora_recv(lora_dev, transmission_data, MAX_TRANSM_LEN, K_MSEC(iteration_time),
										&rssi, &snr);
								printk("remaining iteration time: %lld\n", iteration_time);
								
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
									uint8_t bit_error_count = 0;
									for(int z = 0; z < exp_data[2]; z++) {
										if(compare_data[z] != transmission_data[z]) {
											same_content = false;
											bit_error_count++;
										}
									}


									uint8_t ndata[3] = {0};
									rssi = (uint8_t) -rssi; 											// negated to fit into an unsigned int (original value is negative)
									ndata[0] = rssi;
									ndata[1] = snr;
									ndata[2] = bit_error_count;

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
								printk("millissec: %lld\n", milliseconds_spent);
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
	reconnect = false;
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

	if(config.tx_power > 10) {
		k_sleep(K_MSEC(5000));
		while(!bt_lres_connected) {
			printk("waiting 300 ms\n");
			k_sleep(K_MSEC(300));
		}
		k_sleep(K_MSEC(5000));
	}

	if (l < 0) {
		LOG_ERR("no response received");	
		int8_t bt_data[1] = {-7};
			bt_lres_notify(bt_data, 2);	
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
		if(!reconnect) {						// in case of a pre experiment communication over 10 dBm
			change_config(pu, true);
		}
		reconnect = true;
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
		printk("exp data index%c\n", *pu);
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

	if(type_of_notification == 0) {		// RSSI/SNR notification + number of bit errors in experiment
		uint8_t *pu = (uint8_t *) data;

		static uint8_t stats[4];

		// put -1 at index 0 to distinguish from msg notification
		stats[0] = -1;
		stats[1] = *pu;
		pu++;
		stats[2] = *pu;
		pu++;
		stats[3] = *pu;

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

void set_connected(bool bt_lres_conn) {
	bt_lres_connected = bt_lres_conn;
}

SYS_INIT(lres_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);