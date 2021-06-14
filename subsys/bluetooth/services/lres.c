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
#define MAX_TRANSM_LEN 20

#define LOG_LEVEL CONFIG_BT_LRES_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lres);

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

static uint8_t lres_blsc;
static struct lora_modem_config config;

// when descriptor changed at phone (for enableing notifications)
static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LRES notifications %s", notif_enabled ? "enabled" : "disabled");
}

// implemented at bottom of file (declared here for use in next function)
int bt_lres_notify(const void *data, uint8_t type_of_notification);

void change_config(uint8_t* pu, bool tx) {
	const struct device *lora_dev;
	uint16_t len = 5;

	int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};

	config.frequency = frequencies[*pu];
	printk("[NOTIFICATION] data %d length %u\n", *pu, len);
	pu++;

	config.bandwidth = *pu;
	printk("[NOTIFICATION] data %d length %u\n", *pu, len);
	pu++;

	config.datarate = *pu + 7;
	printk("[NOTIFICATION] dat %d length %u\n", *pu, len);
	pu++;

	config.preamble_len = 8;

	config.coding_rate = *pu + 1;
	printk("[NOTIFICATION] data %d length %u\n", *pu, len);
	pu++;

	config.tx_power = *pu + 5;
	printk("[NOTIFICATION] data %d length %u\n", *pu, len);

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

// gets 5 bytes from phone indicating LoRa configuration settings (callback for the corresponding characteristic)
static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	uint8_t *pu = (uint8_t *) buf;
	change_config(pu, false);

	int8_t bt_data[1] = {-2};
	bt_lres_notify(bt_data, 2);
	return 0;
}


#define STACK_SIZE 8192
#define TT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area, STACK_SIZE);

struct k_thread thread_data;

uint8_t experiment_data[18];
uint16_t exp_data_length = 0;

void exec_experiment(void *a, void *b, void *c) {
	uint8_t exp_data[exp_data_length];
	uint16_t len = exp_data_length;

	for(int16_t i = 0; i < len; i++) {
		exp_data[i] = experiment_data[i];	
	}

	const struct device *lora_dev;
	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	int ret;
	int16_t rssi;
	int8_t snr;
	int l = -1;
	uint8_t data[MAX_DATA_LEN] = {0};					// data array for ACK from sender device
	bool exp_started = false;

	while(!exp_started) {
		// configure as sender and send experiment settings
		config.tx = true;
		ret = lora_config(lora_dev, &config);
		if (ret < 0) {
			LOG_ERR("LoRa config failed");
		}

		ret = lora_send(lora_dev, exp_data, len);
		if (ret < 0) {
			LOG_ERR("LoRa send failed");
		}

		// configure as receiver, wait 2 seconds for ACK
		config.tx = false;
		ret = lora_config(lora_dev, &config);
		if (ret < 0) {
			LOG_ERR("LoRa config failed");
		}
			
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(2),
					&rssi, &snr);
		if (l < 0) {
			LOG_ERR("no ACK received");	
		} else {
			if(memcmp(exp_data, data, len * sizeof(uint8_t)) == 0) {
				printk("ACK is okay...............\n");
				exp_started = true;				// check if received data exactly matches sent data
			}
		}
	}


	char delay[5];				// get experiment start delay
	for(int16_t i = 0; i < l; i++) {
		if(i > 8) {			
			delay[i-9] = exp_data[i];
		}
	}
	uint16_t d = atoi(delay); 


	printk("exp_data[7] (coding rate byte): %d\n",  exp_data[7]);
	

	bool first_iteration = true;								
	uint8_t transmission_data[MAX_TRANSM_LEN] ={0};				// exp_data[2] contains msg length
	config.tx = false;
	int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};

	for(uint8_t i = 0; i < 8; i++) {
		if(((exp_data[4] >> i)  & 0x01) == 1) {					// the 4th byte of the settings byte array represents the frequencies to use
			config.frequency = frequencies[i];					// if a bit in that byte is set, the corresponding frequency will be used;
			printk("frequency: %d\n", frequencies[i]);
		} else {
			continue;
		}

		for(uint8_t j = 0; j < 3; j++) {
			if(((exp_data[5] >> j)  & 0x01) == 1) {
				config.bandwidth =  j;
				printk("bandwidth: %d\n", j);
			} else {
				continue;
			}
			for(uint8_t k = 0; k < 6; k++) {
				if(((exp_data[6] >> k)  & 0x01) == 1) {
					config.datarate =  k + 7;
					printk("data rate: %d\n", k+7);
				} else {
					continue;
				}
				for(uint8_t m = 0; m < 4; m++) {
					if(((exp_data[7] >> m)  & 0x01) == 1) {
						config.coding_rate =  m + 1;
						printk("coding rate: %d\n", m+1);
					} else {
						printk("coding rate bit not set for %d\n", m);
						continue;
					}
					for(uint8_t p = 0; p < 8; p++) {
						if(((exp_data[8] >> p)  & 0x01) == 1) {
							config.tx_power =  p + 5;
							printk("power: %d\n", p+1);
							ret = lora_config(lora_dev, &config);
							if (ret < 0) {
								LOG_ERR("LoRa config failed");
							}

							

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
								l = lora_recv(lora_dev, transmission_data, MAX_DATA_LEN, K_MSEC(iteration_time),
										&rssi, &snr);

								if(last_data_8 != transmission_data[8]) {								// checking if lora_recv just timed out or if something was actually received
									bt_lres_notify(transmission_data, 1);
									LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
										log_strdup(transmission_data), rssi, snr);
									last_data_8 = transmission_data[8];
								}

								milliseconds_spent = k_uptime_delta(&time_stamp);
								time_stamp = k_uptime_get();	
								iteration_time = iteration_time - milliseconds_spent;						
							}

							// k_sleep(K_MSEC(5000));														// wait 5 seconds between combinations
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


uint16_t len_param = 0;

static ssize_t exp_settings_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	printk("at callback\n");

	exp_data_length = len;
	uint8_t *pu = (uint8_t *) buf;
	for(int16_t i = 0; i < len; i++) {
		experiment_data[i] = *pu;
		pu++;
	}

	k_tid_t my_tid = k_thread_create(&thread_data, stack_area,
                                 K_THREAD_STACK_SIZEOF(stack_area),
                                 exec_experiment,
                                 NULL, NULL, NULL,
                                 TT_PRIORITY, 0, K_NO_WAIT);

	printk("my_tid: %d\n", my_tid);


	return 0;
}

// Lora Eval Service Declaration: service, descriptor, 1 notification characteristic, 1 write characteristic
BT_GATT_SERVICE_DEFINE(lres_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_LRES),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_STAT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(lec_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_CHANGE_CONFIG, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, change_config_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LRES_EXP, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, exp_settings_cb, NULL),
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
		printk("at stats\n");
		uint8_t *pu = (uint8_t *) data;

		static uint8_t stats[3];

		// put -1 at index 0 to distinguish from msg notification
		stats[0] = -1;
		stats[1] = *pu;
		pu++;
		stats[2] = *pu;

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &stats, sizeof(stats));
	} else if(type_of_notification == 1){							// msg notification
		printk("at msg\n");
		char *pc = (char *) data;
		char data[MAX_DATA_LEN];
	
		for(uint16_t i = 0; i < MAX_DATA_LEN; i++) {
			data[i] = *pc;
			pc++;
		}

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &data, sizeof(data));
	} else {		// notify about config changed
		static int8_t notifier[1];
		notifier[0] = -2;

		rc = bt_gatt_notify(NULL, &lres_svc.attrs[1], &notifier, sizeof(notifier));
	}
	

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(lres_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);