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

// defining thread related stuff
#define STACK_SIZE 1024
#define TTT_PRIORITY 5
K_THREAD_STACK_DEFINE(stack_area, STACK_SIZE);
struct k_thread thread_data;

uint8_t e_d[18];
uint16_t length = 0;
void testThread1(void *a, void *b, void *c) {
	//uint16_t len = *(uint16_t*) b;
	uint8_t exp_data[length];					// experiment settings data
	for(int16_t i = 0; i < length; i++) {
		exp_data[i] = e_d[i];
		
	}
	for(int16_t j = 0; j < length; j++) {
		printk("sth: %d\n", exp_data[j]);
	}
	return;
}

// is triggered when writing to corresponding characteristic on phone
static ssize_t test_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset, uint8_t sth)
{
	printk("at callback\n");

	length = len;
	uint8_t *pu = (uint8_t *) buf;
	for(int16_t i = 0; i < len; i++) {
		e_d[i] = *pu;
		pu++;
	}

    k_tid_t my_tid = k_thread_create(&thread_data, stack_area,
                                 K_THREAD_STACK_SIZEOF(stack_area),
                                 testThread1,
                                 NULL, NULL, NULL,
                                 TTT_PRIORITY, K_USER, K_NO_WAIT);

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
