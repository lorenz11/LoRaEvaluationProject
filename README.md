# LoRaVisual

# Mobile Evaluation System for LoRa PHY

## Intro

This repo hosts the source code for the embedded part of the evaluation system.
You can find the other to parts if you follow these links:
- source code for Android app:

- source code visualiser and experiment data:
## Abstract

## Implementation

We implement LoRa Visual in C for the RTOS Zephyr.
We support the Adafruit nRF52840 Feather with the LoRa module on an Adafruit FeatherWing.

## Code structure

### Shared between sender and receiver device
Modified devicetree to encorporate Radio FeatherWing (with its LoRa module):  
[boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts](./boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts)  
UUIDs to identify BLE services (lines 326 - 346) and their characteristics (lines 822 - 922):  
[include/bluetooth/uuid.h](./include/bluetooth/uuid.h)  
CMakeLists for BLE sercices:
[subsys/bluetooth/services/CMakeLists.txt](./subsys/bluetooth/services/CMakeLists.txt)

### Source code for sender device
The directory [samples/lora_eval_send](./samples/lora_eval_send) contains the main c file [samples/lora_eval_send/src/main.c](./samples/lora_eval_send/src/main.c) as well as some configuration and build code ([samples/lora_eval_send/CMakeLists.txt](./samples/lora_eval_send/CMakeLists.txt), [samples/lora_eval_send/prj.conf](./samples/lora_eval_send/prj.conf), [samples/lora_eval_send/sample.yaml](./samples/lora_eval_send/sample.yaml))  

The code that defines the BLE characteristics for the sender device and the threads we initiate in their callbacks:
[subsys/bluetooth/services/lses.c](./subsys/bluetooth/services/lses.c)  
Configuration file for the sender's BLE service:
[subsys/bluetooth/services/Kconfig.lses](./subsys/bluetooth/services/Kconfig.lses)  
Header file for the BLE service c code:  
[include/bluetooth/services/lses.h](./include/bluetooth/services/lses.h)  
