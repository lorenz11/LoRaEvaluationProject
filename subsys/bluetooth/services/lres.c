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

#include "payloaddepot.h"

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


// for convenience: change LoRa parameter configuration according to arguments
void change_config(uint8_t* pu, bool tx) {
	const struct device *lora_dev;
	
	config.frequency = frequencies[*pu];
	printk("frequency: %d, ", *pu);
	pu++;

	config.bandwidth = *pu;
	printk("bandwidth: %d, ", *pu);
	pu++;

	config.datarate = *pu + 7;
	printk("spreading factor: %d, ", *pu);
	pu++;

	config.preamble_len = 8;

	config.coding_rate = *pu + 1;
	printk("coding rate %d, ", *pu);
	pu++;

	config.tx_power = 18 - (*pu * 2);
	printk("power: %d dBm\n", *pu);

	config.tx = tx;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}
	lora_init(lora_dev);

	int ret;
	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}
}

// implemented at bottom of file (declared here for use in next function)
int bt_lres_notify(const void *data, uint8_t type_of_notification);

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
			lora_send(lora_dev, data, len);		// respond to ping
			config.tx = false;
			lora_config(lora_dev, &config);	
			continue;
		}
		
		uint8_t ndata[3] = {0};
		rssi = (uint8_t) -rssi; // negated to fit into an unsigned byte for phone notification (original value is negative)
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
	
	if(thread0_tid != NULL) {
		k_thread_abort(thread0_tid);
	}

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


K_THREAD_STACK_DEFINE(stack_area1, STACK_SIZE);

struct k_thread thread_data1;
k_tid_t thread1_tid;

uint8_t experiment_data[18];
uint16_t exp_data_length = 0;

// for handling the high dBm LoRa/ble connection loss problem
bool reconnect = false;

// experiment receive thread code
void exec_experiment(void *a, void *b, void *c) {
	// initialize stuff
	uint8_t iteration = 0;
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
		ret = lora_send(lora_dev, exp_data, len);						// send experiment settings to other board

		config.tx = false;
		ret = lora_config(lora_dev, &config);							// configure as receiver
			
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(2),		// wait 2 seconds for ACK
					&rssi, &snr);
		if (l < 0) {
			LOG_ERR("no ACK received.");	
			int8_t bt_data[1] = {-5};
			bt_lres_notify(bt_data, 2);	
		} else {
			if(memcmp(exp_data, data, len * sizeof(uint8_t)) == 0) {	// check if received ACK exactly matches sent data (experiment parameters)
				printk("ACK is okay\n");
				int8_t bt_data[1] = {-6};
				bt_lres_notify(bt_data, 2);	
				exp_started = true;									
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
		d -= 17;
		k_sleep(K_MSEC(17000));
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
		if(((exp_data[4] >> i)  & 0x01) == 1) {					
			lora_init(lora_dev);
			config.frequency = frequencies[i];					// the 4th byte of the settings byte array represents the frequencies to use
			compare_data[0] = (char) i + 48;					// if a bit in that byte is set, the corresponding frequency will be used;
			printk("frequency: %d, ", frequencies[i]);			// same for all other LoRa parameters
		} else {
			continue;
		}

		for(uint8_t j = 0; j < 3; j++) {
			if(((exp_data[5] >> j)  & 0x01) == 1) {
				config.bandwidth =  j;
				compare_data[1] = (char) j + 48;
				printk("bandwidth: %d, ", j);
			} else {
				continue;
			}
			for(uint8_t k = 0; k < 6; k++) {
				if(((exp_data[6] >> k)  & 0x01) == 1) {
					config.datarate =  k + 7;
					compare_data[2] = (char) k + 48;
					printk("data rate: %d, ", k+7);
				} else {
					continue;
				}
				for(uint8_t m = 0; m < 4; m++) {
					if(((exp_data[7] >> m)  & 0x01) == 1) {
						config.coding_rate =  m + 1;
						compare_data[3] = (char) m + 48;
						printk("coding rate: %d, ", m+1);
					} else {
						continue;
					}
					for(uint8_t p = 0; p < 8; p++) {
						if(((exp_data[8] >> p)  & 0x01) == 1) {
							config.tx_power =  18 - (p * 2);
							compare_data[4] = (char) p + 48;
							printk("power: %d dBm\n", 18 - (p * 2));


							// start of an iteration: configure LoRa parameters and initialize timing
							ret = lora_config(lora_dev, &config);
							
							int64_t time_stamp;
							int64_t milliseconds_spent = 0;
							int64_t millis_total = 200;
							time_stamp = k_uptime_get();
							int64_t iteration_time = exp_data[0] * exp_data[1] * 1000;					// exp_data[0] * exp_data[1] = # LoRa transmissions * time between transmissions
							
							if(first_iteration) {
								iteration_time += (1000 * d);											// count down delay as part of the LoRa receive timing
							}
							iteration_time += 5000 ;													// delay between iterations as part of the LoRa receive timing
							printk("time for this iteration: %lld\n", iteration_time);					


							// time-subtracting-down-to-zero-every-time-something-was-received-loop
							while(iteration_time > 0) {												
								l = lora_recv(lora_dev, transmission_data, MAX_TRANSM_LEN, K_MSEC(iteration_time),
										&rssi, &snr);

								// handling timing after message
								milliseconds_spent = k_uptime_delta(&time_stamp);
								time_stamp = k_uptime_get();	
								iteration_time = iteration_time - milliseconds_spent;					// control iteration time to know when to switch LoRa parameters for next iteration	
								printk("!new round!\nremaining iteration time: %lld\n", iteration_time);
								printk("millissec: %lld\n", milliseconds_spent);
								millis_total += milliseconds_spent;
								

								// if something was actually received (and not lora_recv just timed out)
								if(l >= 0) {
									LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
										log_strdup(transmission_data), rssi, snr);

									// determine the message number condsidering the time
									uint16_t msg_number = (first_iteration ? 										
											(millis_total - (1000 * d) - 5000) : (millis_total - 5000))
												/ (exp_data[1] * 1000);
									printk("msg_number: %d\n", msg_number);
									printk("millis total: %lld\n", millis_total);


									// determine what the received message SHOULD contain (from the 7th byte, rest was done before)
									// message number part of content
									compare_data[6] = (char) msg_number / 100 + 48;				
									compare_data[7] = (char) msg_number / 10 + 48;						
									compare_data[8] = (char) msg_number % 10 + 48;

									// payload part of the content
									for(uint8_t p = 9; p < (exp_data[2] - 1); p++) {					// exp_data[2] contains message length (length of the transmitted content)
										compare_data[p] = random_d[(msg_number 
											* (exp_data[2] - 9) + (p - 9)) % 200];						// use the msg number to determine which random data the transmission should contain		
									}																	// fills the message up with with random payload data until desired message length
									compare_data[exp_data[2] - 1] = '.';								// msg delimiter


									// check the content of received message and calculate bit error rate
									bool same_content = true;
									uint8_t bit_error_count = 0;
									for(int z = 0; z < exp_data[2]; z++) {
										if(compare_data[z] != transmission_data[z]) {
											same_content = false;
											for(int i = 0; i < 8; i++) {	
												if(((compare_data[z] >> i) & 0x01) != ((transmission_data[z] >> i) & 0x01)) {
													bit_error_count++;
												}
											}
										}
									}


									// notify phone about the results
									uint8_t ndata[3] = {0};
									rssi = (uint8_t) -rssi; 											// negated to fit into an unsigned int (original value is negative)
									ndata[0] = rssi;
									ndata[1] = snr;
									ndata[2] = bit_error_count;

									bt_lres_notify(ndata, 0);
									if(same_content) {
										transmission_data[9] = 't';										// indicating to phone if received content was correct ( by adding a t or f to the header)
									} else {
										transmission_data[9] = 'f';
									}
									
									transmission_data[10] = iteration;
									printk("iteration %d\n", iteration);
									transmission_data[11] = msg_number; 								// send correct message number to phone as byte (rest as char[])
									transmission_data[12] = '.';	

									bt_lres_notify(transmission_data, 1);								// send results to phone to monitor experiment
								}					
							}
							if(first_iteration)
								first_iteration = false;
							iteration++;
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



// for pings previous to experiment start
#define STACK_SIZE2 2048
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area2, STACK_SIZE2);

struct k_thread thread_data2;
k_tid_t thread2_tid;

// function for waiting for ping return on separate thread
void wait_for_ping_return(void *a, void *b, void *c) {
	int16_t rssi;
	int8_t snr;
	int l = -1;
	char ping[5] = {'!', 'p', 'i', 'n', 'g'};							// ping content

	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);
	lora_send(lora_dev, ping, 5);										// send ping					

	config.tx = false;
	lora_config(lora_dev, &config);						
		
	char resp[5] = {0};
	l = lora_recv(lora_dev, resp, MAX_DATA_LEN, K_SECONDS(3),			// wait 3 seconds for response
				&rssi, &snr);

	if(config.tx_power > 10) {											// waiting for reconnection to phone for notification after disconnect (only necessary for high dbm lora pings)
		k_sleep(K_MSEC(5000));
		while(!bt_lres_connected) {
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
			printk("ping does not match: %s\n", resp);	
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
		if(thread2_tid != NULL) {
			k_thread_abort(thread2_tid);
		}

		thread2_tid = k_thread_create(&thread_data2, stack_area2,
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
	

	if(thread1_tid != NULL) {
		k_thread_abort(thread1_tid);
	}

	thread1_tid = k_thread_create(&thread_data1, stack_area1,
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