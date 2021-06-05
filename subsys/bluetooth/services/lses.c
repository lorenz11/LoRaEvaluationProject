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

#define MAX_DATA_LEN 30

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


// when descriptor changed at phone (for enableing notifications)
static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LSES notifications %s", notif_enabled ? "enabled" : "disabled");
}

// implemented at bottom of file (declared here for use in nexxt function)
int bt_lses_notify(int8_t type_of_notification);

// gets 5 bytes from phone indicating LoRa configuration settings (callback for the corresponding characteristic)
static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	const struct device *lora_dev;
	struct lora_modem_config config;
	int ret;

	uint8_t *pu = (uint8_t *) buf;
	int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};

	config.frequency = frequencies[*pu];
	pu++;

	config.bandwidth = *pu;
	pu++;

	config.datarate = *pu + 7;
	pu++;

	config.preamble_len = 8;

	config.coding_rate = *pu + 1;
	pu++;

	config.tx_power = *pu + 5;

	config.tx = true;

 	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}

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

	if(*pc == 'b') {			// set number of loops in explore mode
		struct lora_modem_config config;

		config.frequency = 868100000;
		config.bandwidth = 0;
		config.datarate = 10;
		config.preamble_len = 8;
		config.coding_rate = 1;
		config.tx_power = 5;
		config.tx = false;

		ret = lora_config(lora_dev, &config);
		if (ret < 0) {
			LOG_ERR("LoRa config failed");
		}


		int16_t rssi;
		int8_t snr;
		int le;
		le = lora_recv(lora_dev, data, MAX_DATA_LEN, K_FOREVER,
					&rssi, &snr);
			if (len < 0) {
				LOG_ERR("LoRa receive failed");
				return 0;
			}

		printk("printedddd: %s\n", log_strdup(data));
		LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
				log_strdup(data), rssi, snr);
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
