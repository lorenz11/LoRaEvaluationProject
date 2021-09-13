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
[zephyr/boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts](.zephyr/boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts)

