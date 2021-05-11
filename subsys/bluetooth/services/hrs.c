/** @file
 *  @brief HRS Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
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

#define LOG_LEVEL CONFIG_BT_HRS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(hrs);

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

static uint8_t hrs_blsc;

static void hrmc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("HRS notifications %s", notif_enabled ? "enabled" : "disabled");
}

static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, uint16_t len, uint16_t offset)
{
	char data[100];
	strcpy(data, attr->user_data);
	printk("thiss: %s\n", data);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &hrs_blsc,
				 sizeof(hrs_blsc));
}

//'ssize_t (*)(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t,  uint16_t,  uint8_t)'

static ssize_t testt_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	uint8_t *pu = (uint8_t *) buf;
	//pu++;
	uint32_t frequ = 0;
	frequ += (*pu) * 100000000;
	pu++;
	frequ += (*pu) * 10000000;
	pu++;
	frequ += (*pu) * 1000000;

 

	const struct device *lora_dev;
	struct lora_modem_config config;
	int ret;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
	}

	config.frequency = frequ;
	config.bandwidth = BW_125_KHZ;
	config.datarate = SF_10;
	config.preamble_len = 8;
	config.coding_rate = CR_4_5;
	config.tx_power = 4;
	config.tx = true;

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
	}



	printk("[NOTIFICATION] data %d length %u\n", *pu, len);
	return 0;
}

/* Heart Rate Service Declaration */
BT_GATT_SERVICE_DEFINE(hrs_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_HRS),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_MEASUREMENT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(hrmc_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_BODY_SENSOR, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_blsc, NULL, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_TESTT, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, testt_cb, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_CONTROL_POINT, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
);

static int hrs_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	hrs_blsc = 0x01;

	return 0;
}

int bt_hrs_notify(uint16_t heartrate)
{
	int rc;
	static uint8_t hrm[2];

	hrm[0] = 0x06; /* uint8, sensor contact */
	hrm[1] = heartrate;

	rc = bt_gatt_notify(NULL, &hrs_svc.attrs[1], &hrm, sizeof(hrm));

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(hrs_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
