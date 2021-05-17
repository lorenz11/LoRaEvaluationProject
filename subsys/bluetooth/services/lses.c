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

static void lec_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("LSES notifications %s", notif_enabled ? "enabled" : "disabled");
}


static ssize_t change_config_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	const struct device *lora_dev;
	struct lora_modem_config config;
	int ret;

	uint8_t *pu = (uint8_t *) buf;
	if(*pu == 0) {
		config.frequency = 865000000;
	} else if (*pu == 1) {
		config.frequency = 868000000;
	} else {
		config.frequency = 871000000;
	}
	pu++;

	if(*pu == 0) {
		config.bandwidth = BW_125_KHZ;
	} else if (*pu == 1) {
		config.bandwidth = BW_250_KHZ;
	} else {
		config.bandwidth = BW_500_KHZ;
	}
	pu++;

	if(*pu == 0) {
		config.datarate = SF_6;
	} else if (*pu == 1) {
		config.datarate = SF_7;
	} else if (*pu == 2) {
		config.datarate = SF_8;
	}else if (*pu == 3) {
		config.datarate = SF_9;
	}else if (*pu == 4) {
		config.datarate = SF_10;
	}else if (*pu == 5) {
		config.datarate = SF_11;
	}else {
		config.datarate = SF_12;
	}
	pu++;

	config.preamble_len = 8;

	if(*pu == 0) {
		config.coding_rate = CR_4_5;
	} else if (*pu == 1) {
		config.coding_rate = CR_4_6;
	} else if (*pu == 2) {
		config.coding_rate = CR_4_7;
	} else {
		config.coding_rate = CR_4_8;
	}
	pu++;

	if(*pu == 0) {
		config.tx_power = 4;
	} else if (*pu == 1) {
		config.tx_power = 5;
	} else if (*pu == 2) {
		config.tx_power = 6;
	}else if (*pu == 3) {
		config.tx_power = 7;
	}else if (*pu == 4) {
		config.tx_power = 8;
	}else if (*pu == 5) {
		config.tx_power = 9;
	}else if (*pu == 6) {
		config.tx_power = 10;
	}else if (*pu == 7) {
		config.tx_power = 11;
	}else {
		config.tx_power = 12;
	}
	config.tx = true;

 	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}

	printk("[NOTIFICATION] data %d length %u\n", *pu, len);
	return 0;
}

static ssize_t send_command_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	char *pc = (char *) buf;
	char data[MAX_DATA_LEN];
	
	for(uint16_t i = 0; i < len; i++) {
		data[i] = *pc;
		pc++;
	}
	printk("msg: %s", data);

	const struct device *lora_dev;
	int ret;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	ret = lora_send(lora_dev, data, MAX_DATA_LEN);
	if (ret < 0) {
		LOG_ERR("LoRa send failed");
		return 0;
	
	}
	return 0;
}

/* Lora Eval Service Declaration */
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
);

static int lses_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	lses_blsc = 0x01;

	return 0;
}

// for future notifications
/*int bt_lses_notify(uint16_t data)
{
	rc = bt_gatt_notify(NULL, &lses_svc.attrs[1], &d, sizeof(d));
	return rc == -ENOTCONN ? 0 : rc;
}*/

SYS_INIT(lses_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
