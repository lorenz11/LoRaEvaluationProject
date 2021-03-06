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

#define MAX_DATA_LEN 255

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(lora_receive);
struct lora_modem_config config;

void main(void)
{
	const struct device *lora_dev;
	int len;
	uint8_t data[MAX_DATA_LEN] = {0};
	int16_t rssi;
	int8_t snr;

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
		return;
	}
	int frequencies[8] =  {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 869500000};


	
	bool sixEight = true;
	while (1) {
		if(sixEight) {
			config.frequency = frequencies[7];
			config.bandwidth = BW_125_KHZ;
			config.datarate = SF_10;
			config.preamble_len = 8;
			config.coding_rate = CR_4_5;
			config.tx_power = 5;
			config.tx = false;

			lora_config(lora_dev, &config);
			sixEight = false;
		} else {
			config.frequency = frequencies[7];
			config.bandwidth = BW_125_KHZ;
			config.datarate = SF_10;
			config.preamble_len = 8;
			config.coding_rate = CR_4_5;
			config.tx_power = 5;
			config.tx = false;

			lora_config(lora_dev, &config);
			sixEight = true;
		}

		len = lora_recv(lora_dev, data, MAX_DATA_LEN, K_FOREVER,
				&rssi, &snr);
		if (len < 0) {
			LOG_ERR("LoRa receive failed");
			return;
		}

		printk("sth\n");
		LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)",
			log_strdup(data), rssi, snr);

		
	}
}