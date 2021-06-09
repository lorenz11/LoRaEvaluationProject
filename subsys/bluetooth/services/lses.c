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

#define MAX_DATA_LEN 20

#define LOG_LEVEL CONFIG_BT_LSES_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lses);

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

static uint8_t lses_blsc;

// to control the sending LoRa messages loop
static uint8_t loop = 0;
static uint16_t number_of_messages = 0;

static struct lora_modem_config config;


// when descriptor changed at phone (for enableing notifications)
static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LSES notifications %s", notif_enabled ? "enabled" : "disabled");
}

// implemented at bottom of file (declared here for use in nexxt function)
int bt_lses_notify(int8_t type_of_notification);

void change_config(uint8_t* pu, bool tx) {
	const struct device *lora_dev;

	int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};

	config.frequency = frequencies[*pu];
	printk("[NOTIFICATION] data %d\n", *pu);
	pu++;

	config.bandwidth = *pu;
	printk("[NOTIFICATION] data %d length\n", *pu);
	pu++;

	config.datarate = *pu + 7;
	printk("[NOTIFICATION] dat %d length\n", *pu);
	pu++;

	config.preamble_len = 8;

	config.coding_rate = *pu + 1;
	printk("[NOTIFICATION] data %d length\n", *pu);
	pu++;

	config.tx_power = *pu + 5;
	printk("[NOTIFICATION] data %d length\n", *pu);

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
	change_config(pu, true);

	bt_lses_notify(-2);
	return 0;
}

// gets a byte array containing the msg a LoRa transmission is supposed to contain and an optional number of loops for the explore mode
static ssize_t send_command_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	uint8_t single_msg = 0;

	// buf has '!' at index 0 if messages are supposed to be sent in a loop
	char *pc = (char *) buf;
	if(*pc == 33) {
		single_msg = 1;
		loop = 1;
		pc++;
	}

	// extract LoRa message
	printk("length: %d\n", len);
	char data[len];
	
	for(uint16_t i = 0; i < len; i++) {
		data[i] = *pc;
		pc++;
	}
	data[len] = '.';
	printk("msg: %s\n", data);

	const struct device *lora_dev;
	int ret;

	// send message (single or in loop depending on <single_msg>)
	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	if(single_msg == 1) {
		uint16_t i = 0;
		while (loop && (i < number_of_messages)) {
			ret = lora_send(lora_dev, data, MAX_DATA_LEN);
			if (ret < 0) {
				LOG_ERR("LoRa send failed");
				return 0;
			}
			LOG_INF("Data sent!");
			k_sleep(K_MSEC(2000));
			i++;
		}
	} else {
		ret = lora_send(lora_dev, data, MAX_DATA_LEN);
		if (ret < 0) {
			LOG_ERR("LoRa send failed");
			return 0;
		}
	}
	return 0;
}

// callback for characteristic possibly used in the future
static ssize_t anything_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	printk("command received\n");
	char *pc = (char *) buf;
	if(*pc == 'a') {			// set number of loops in explore mode
		loop = 0;
		pc++;
		uint16_t i = atoi(pc);
		number_of_messages = i;
		printk("number: %d\n", i);		
	}

	if(*pc == 'b') {
		// configure and prepare experiment mode
		const struct device *lora_dev;
		lora_dev = device_get_binding(DEFAULT_RADIO);
		if (!lora_dev) {
			LOG_ERR("%s Device not found", DEFAULT_RADIO);
		}

		pc++;
		change_config(pc, false);
		bt_lses_notify(-2);

		printk("sender ready\n");




		// wait for experiment started notification
		int16_t rssi;
		int8_t snr;
		int l;
		uint8_t data[MAX_DATA_LEN] = {0};
		l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_FOREVER,
					&rssi, &snr);
		if (l < 0) {
			LOG_ERR("LoRa receive failed");
			return 0;
		}
		
		char delay[l-9];
		for(int16_t i = 0; i < l; i++) {
			printk("unpacked data: %d\n", data[i]);
			if(i > 8) {			// get experiment start delay
				delay[i-9] = data[i];
			}
		}
		uint16_t d = atoi(delay); 
		printk("delay: %d\n", d);




		// reconfigure device for sending and send received data as ACK
		bool experiment_ready = false;
		while(!experiment_ready) {
			experiment_ready = true;
			int ret;

			config.tx = true;
			ret = lora_config(lora_dev, &config);
			if (ret < 0) {
				LOG_ERR("LoRa config failed");
			}
			k_sleep(K_MSEC(200));
			ret = lora_send(lora_dev, data, MAX_DATA_LEN);
			if (ret < 0) {
				LOG_ERR("LoRa send failed");
				return 0;
			}



			config.tx = false;
			ret = lora_config(lora_dev, &config);
			if (ret < 0) {
				LOG_ERR("LoRa config failed");
			}
			l = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(d),
					&rssi, &snr);
			if (l < 0) {
				LOG_ERR("LoRa receive failed");
			}
		}




		// start experiment
		char transmission_data[data[2]]; 	// data[2] contains msg length
		transmission_data[5] = '_';
	
		config.tx = true;
		int ret;
		int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};
		for(uint8_t i = 0; i < 8; i++) {
			if(((data[4] >> i)  & 0x01) == 1) {					// the 4th byte of the settings byte array represents the frequencies to use
				config.frequency = frequencies[i];				// if a bit in that byte is set, the corresponding frequency will be used;
				transmission_data[0] = (char) i + 48;
			} else {
				continue;
			}

			for(uint8_t j = 0; j < 3; j++) {
				if(((data[5] >> j)  & 0x01) == 1) {
					config.bandwidth =  j;
					transmission_data[1] = (char) j + 48;
				} else {
					continue;
				}
				for(uint8_t k = 0; k < 6; k++) {
					if(((data[6] >> k)  & 0x01) == 1) {
						config.datarate =  k + 7;
						transmission_data[2] = (char) k + 48;
					} else {
						continue;
					}
					for(uint8_t l = 0; l < 4; l++) {
						if(((data[7] >> l)  & 0x01) == 1) {
							config.coding_rate =  l + 1;
							transmission_data[3] = (char) l + 48;
						} else {
							continue;
						}
						for(uint8_t m = 0; m < 8; m++) {
							if(((data[8] >> m)  & 0x01) == 1) {
								config.tx_power =  m + 1;
								transmission_data[4] = (char) m + 48;
								ret = lora_config(lora_dev, &config);
								if (ret < 0) {
									LOG_ERR("LoRa config failed");
								}
								for(uint8_t n = 0; n < data[0]; n++) {						// data[0] contains the number of LoRa transmissions per parameter combination
									transmission_data[6] = (char) n / 100 + 48;						// include numbering into transmission content (as String (3 bytes) not as byte (1 byte))
									transmission_data[7] = (char) n / 10 + 48;						// this line needs to be changed if max number of transmissions per combination is changed (it is 100 now)
									transmission_data[8] = (char) n % 10 + 48;

									for(uint8_t p = 9; p < data[2]; p++) {					// data[2] contains message length (length of the transmitted content)
										transmission_data[p] = 'a';								// fills the message up with a's until desired message length
									}

									ret = lora_send(lora_dev, transmission_data, 11);
									if (ret < 0) {
										LOG_ERR("LoRa send failed");
										return 0;
									}
									k_sleep(K_MSEC(data[1] * 1000));						// data[1] contains the number of seconds between transmissions
								}

								k_sleep(K_MSEC(5000));										// wait 5 seconds between combinations
							} else {
								continue;
							}
						}
					}
				}
			}
		}
	}
	
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
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_ANYTHING, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, anything_cb, NULL),
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
