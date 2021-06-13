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

#define LOG_LEVEL CONFIG_BT_TESTS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(tests);

static uint8_t lses_blsc;


// is triggered when writing to corresponding characteristic on phone
static ssize_t test_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	printk("at callback\n");
	return 0;
}

// itialization of a minimal BLE service
BT_GATT_SERVICE_DEFINE(lses_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_LSES),
	BT_GATT_CHARACTERISTIC(BT_UUID_LSES_ANYTHING, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, test_cb, NULL),
);

static int lses_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	lses_blsc = 0x01;
	return 0;
}

SYS_INIT(lses_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
