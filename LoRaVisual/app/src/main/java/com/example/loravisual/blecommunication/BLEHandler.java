package com.example.loravisual.blecommunication;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;

/**
 * This class wraps BLE connection related user actions.
 */
public class BLEHandler {
    private static final String TAG = "at BLEHandler";

    // devices
    public static final int SENDER = 1;
    public static final int RECEIVER = 2;
    // modes
    public static final int EXPLORE = 1;
    public static final int EXPERIMENT = 2;

    private BluetoothAdapter bluetoothAdapter;
    private Scanner scanner;
    Context context;

    /**
     * The constructor of this class initialises the Scanner.
     * @param context
     * @param device
     * @param mode
     */
    public BLEHandler(Context context, int device, int mode) {
        this.context = context;
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (!bluetoothAdapter.isEnabled()) {
            bluetoothAdapter.enable();
        }
        scanner = new Scanner(context, bluetoothAdapter, device, mode);
    }

    /**
     * starts the scanning for suitable advertisments. See @Scanner for subsequent actions.
     */
    public void connect() {
        scanner.startScanning();
    }

    /**
     * close a BLE connection once it is not needed anymore.
     */
    public void closeConnection() {
        scanner.closeConnection();
        scanner.stopScanning();
    }

    /**
     * The following five methods expose the smartphone-LoRa device data flow functionality,
     * defined in @Central
     * @param configBytes
     */
    public void writeChangeConfigCharacteristic(byte[] configBytes) {
        scanner.getCentral().writeChangeConfigCharacteristic(configBytes);
    }

    public void writeSendCommandCharacteristic(String loraMsg) {
        scanner.getCentral().writeSendCommandCharacteristic(loraMsg);
    }

    public void writeExperimentSettingsCharacteristic(byte[] experimentSettings) {
        scanner.getCentral().writeExperimentSettingsCharacteristic(experimentSettings);
    }

    public void writeLoopCommandCharacteristic(String s) {
        scanner.getCentral().writeLoopCommandCharacteristic(s);
    }

    public void writePrepareSenderCharacteristic(byte[] bytes) {
        scanner.getCentral().writePrepareSenderCharacteristic(bytes);
    }



    // for possible future use
    public void stopScanning() {
        scanner.stopScanning();
    }

}
