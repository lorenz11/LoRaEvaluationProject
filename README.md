# Mobile Evaluation System for LoRa PHY

## Intro

This repository hosts the source code for a mobile evaluation system for LoRa.
It contains code for two LoRa devices, code for an Android app for monitoring 
and control of the LoRa devices, and a Python script, for data visualisation.

## Code structure
## Code for the LoRa devices

### Implementation

We implement LoRa Visual in C for the RTOS Zephyr.
We support the Adafruit nRF52840 Feather with the LoRa module on an Adafruit FeatherWing.

### Shared between sender and receiver device
Modified devicetree to encorporate Radio FeatherWing (with its LoRa module):  
[boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts](./boards/arm/adafruit_feather_nrf52840/adafruit_feather_nrf52840.dts)  
UUIDs to identify BLE services (lines 326 - 346) and their characteristics (lines 822 - 922):  
[include/bluetooth/uuid.h](./include/bluetooth/uuid.h)  
CMakeLists for BLE services:
[subsys/bluetooth/services/CMakeLists.txt](./subsys/bluetooth/services/CMakeLists.txt)

### Source code for sender device
The directory [samples/lora_eval_send](./samples/lora_eval_send) contains the main c file [samples/lora_eval_send/src/main.c](./samples/lora_eval_send/src/main.c) as well as some configuration and build code ([samples/lora_eval_send/CMakeLists.txt](./samples/lora_eval_send/CMakeLists.txt), [samples/lora_eval_send/prj.conf](./samples/lora_eval_send/prj.conf), [samples/lora_eval_send/sample.yaml](./samples/lora_eval_send/sample.yaml))  

The code that defines the BLE characteristics for the sender device and the threads we initiate in their callbacks:  
[subsys/bluetooth/services/lses.c](./subsys/bluetooth/services/lses.c)  
Configuration file for the sender's BLE service:  
[subsys/bluetooth/services/Kconfig.lses](./subsys/bluetooth/services/Kconfig.lses)  
Header file for the BLE service c code:  
[include/bluetooth/services/lses.h](./include/bluetooth/services/lses.h)  

### Source code for receiver device
The directory [samples/lora_eval_rcv](./samples/lora_eval_rcv) contains the main c file [samples/lora_eval_rcv/src/main.c](./samples/lora_eval_rcv/src/main.c) as well as some configuration and build code ([samples/lora_eval_rcv/CMakeLists.txt](./samples/lora_eval_rcv/CMakeLists.txt), [samples/lora_eval_rcv/prj.conf](./samples/lora_eval_rcv/prj.conf), [samples/lora_eval_rcv/sample.yaml](./samples/lora_eval_rcv/sample.yaml))  

The code that defines the BLE characteristics for the receiver device and the threads we initiate in their callbacks:  
[subsys/bluetooth/services/lres.c](./subsys/bluetooth/services/lres.c)  
Configuration file for the receiver's BLE service:  
[subsys/bluetooth/services/Kconfig.lres](./subsys/bluetooth/services/Kconfig.lres)  
Header file for the BLE service c code:  
[include/bluetooth/services/lres.h](./include/bluetooth/services/lres.h) 

### Build and flash
We use west to build and flash. 
  
Build the sender application:  
west build -p auto -b adafruit_feather_nrf52840 samples/lora_eval_send  
  
Build the receiver application:  
west build -p auto -b adafruit_feather_nrf52840 samples/lora_eval_rcv  
  
Flash the devices:
west flash


## LoRaVisual (Android app)
### Implementation

We implement this part mostly in Java. Screen content is defined in XML.
The app supports Android version 6.0. 
You can find all relevant code at [LoRaVisual/app/src/main](./LoRaVisual/app/src/main).
All code files contain extensive comments.  
To make the app work (automatic upload of experiment results to Firebase), you need to open a Firebase account, generate a key and include
the appropriate configuration file in the code. More information can be found at the Firebase
webpage.  
You also need to get an API key for openweathermap (to automatically download weather data for the time of the experiment). You need to include the key in
[this file](./LoRaVisual/app/src/main/java/com/example/loravisual/screens/ExperimentScreens/StartExperimentScreen.java)
in line 529.


### Java code
This directory contains the five packages: [LoRaVisual/app/src/main/java/com/example/loravisual](./LoRaVisual/app/src/main/java/com/example/loravisual)

#### async
[LoRaVisual/app/src/main/java/com/example/loravisual/async/](./LoRaVisual/app/src/main/java/com/example/loravisual/async/) contains code for asynchrounous actions like the upload of experiment results or the download of weather data.

#### blecommunication
[LoRaVisual/app/src/main/java/com/example/loravisual/blecommunication](./LoRaVisual/app/src/main/java/com/example/loravisual/blecommunication) contains code that enables the app to communicate with the LoRa devices.

#### database
[LoRaVisual/app/src/main/java/com/example/loravisual/database](./LoRaVisual/app/src/main/java/com/example/loravisual/database) handles the access to the app's SQLite database. Its purpose is to store experiment information and results and to create corresponding files.

#### lvutilities
[LoRaVisual/app/src/main/java/com/example/loravisual/lvutilities](./LoRaVisual/app/src/main/java/com/example/loravisual/lvutilities) contains code to access and utilise certain smartphone functionality, like GPS data for localisation or the Accelerometer for logging antenna orientation. It also handles the access to Firebase.

#### screens
This package contains two more packages. It defines the interaction between user and app from several screens. It can be found [here](./app/src/main/java/com/example/loravisual/screens).
##### ExperimentScreens
[LoRaVisual/app/src/main/java/com/example/loravisual/screens/ExperimentScreens](./LoRaVisual/app/src/main/java/com/example/loravisual/screens/ExperimentScreens) defines all user interaction from the creation of an experiment to its execution.

##### ExploreScreens
[LoRaVisual/app/src/main/java/com/example/loravisual/screens/ExploreScreens](./LoRaVisual/app/src/main/java/com/example/loravisual/screens/ExploreScreens) defines all user interaction for the explore mode of the app.

### XML code
The files at [LoRaVisual/app/src/main/res/layout](./LoRaVisual/app/src/main/res/layout) define the screen composition.

## LoraVisualGraphs (Python script for data evaluation)

### Implementation

This is a Python script. To automatically download new experiment data, include the configuration file for your Firebase account, and change the lines 17 - 26 as instructed in [this file](./LoRaVisualGraphs/main.py). Otherwise the script won't start.

### The script
[LoRaVisualGraphs/main.py](./LoRaVisualGraphs/main.py) contains code that starts a GUI from which a user can choose between different graphs and charts for the experiments it automatically gets from Firebase on start.

### Experiment results
[LoRaVisualGraphs/experiment_data](./LoRaVisualGraphs/experiment_data) contains all raw experiment information and results.

