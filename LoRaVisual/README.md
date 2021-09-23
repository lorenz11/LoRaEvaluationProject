# LoRaVisual

# Mobile Evaluation System for LoRa PHY

## Intro

This repo hosts the source code for the Android app of the evaluation system.
All code files contain extensive comments.
You can find the other two parts if you follow these links:
- source code for microcontroller:

- source code visualiser and experiment data:

## Implementation

We implement this part mostly in Java. Screen content is defined in XML.
The app supports Android version 6.0.

## Code structure
You can find all relevant code at [app/src/main](./app/src/main).

### Java code
This directory contains the five packages: [app/src/main/java/com/example/loravisual](./app/src/main/java/com/example/loravisual)

#### async
[app/src/main/java/com/example/loravisual/async/](./app/src/main/java/com/example/loravisual/async/) contains code for asynchrounous actions like the upload of experiment results or the download of weather data.

#### blecommunication
[app/src/main/java/com/example/loravisual/blecommunication](./app/src/main/java/com/example/loravisual/blecommunication) contains code that enables the app to communicate with the LoRa devices.

#### database
[app/src/main/java/com/example/loravisual/database](./app/src/main/java/com/example/loravisual/database) handles the access to the app's SQLite database. Its purpose is to store experiment information and results and to create corresponding files.

#### lvutilities
[app/src/main/java/com/example/loravisual/lvutilities](./app/src/main/java/com/example/loravisual/lvutilities) contains code to access and utilise certain smartphone functionality, like GPS data for localisation or the Accelerometer for logging antenna orientation. It also handles the access to Firebase.

#### screens
This package contains two more packages. It defines the interaction between user and app from several screens. It can be found [here](./app/src/main/java/com/example/loravisual/screens).
##### ExperimentScreens
[app/src/main/java/com/example/loravisual/screens/ExperimentScreens](./app/src/main/java/com/example/loravisual/screens/ExperimentScreens) defines all user interaction from the creation of an experiment to its execution.

##### ExploreScreens
[app/src/main/java/com/example/loravisual/screens/ExploreScreens](./app/src/main/java/com/example/loravisual/screens/ExploreScreens) defines all user interaction for the explore mode of the app.

### XML code
The files at [app/src/main/res/layout](./app/src/main/res/layout) define the screen composition.


