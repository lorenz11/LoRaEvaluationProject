/*
 * Copyright (c) 2019 Manivannan Sadhasivam
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/lora.h>
#include <errno.h>
#include <sys/util.h>
#include <zephyr.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

#define MAX_DATA_LEN 30

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lora_send);

char data[MAX_DATA_LEN] = {'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', '.','h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', '.', 'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', '.'};

void main(void)
{
	printk("started lora send...\n");
	const struct device *lora_dev;
	struct lora_modem_config config;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
		return;
	}

	
	bool sixEight = true;
	while (1) {
		if(sixEight) {
			config.frequency = 869500000;
			config.bandwidth = BW_125_KHZ;
			config.datarate = SF_10;
			config.preamble_len = 8;
			config.coding_rate = CR_4_5;
			config.tx_power = 5;
			config.tx = true;

			lora_config(lora_dev, &config);
			sixEight = false;
		} else {
			config.frequency = 869500000;
			config.bandwidth = BW_125_KHZ;
			config.datarate = SF_10;
			config.preamble_len = 8;
			config.coding_rate = CR_4_5;
			config.tx_power = 5;
			config.tx = true;

		lora_config(lora_dev, &config);
		sixEight = true;
		}

		k_sleep(K_MSEC(3000));

		lora_send(lora_dev, data, MAX_DATA_LEN);

		LOG_INF("Data sent!");
	}
}