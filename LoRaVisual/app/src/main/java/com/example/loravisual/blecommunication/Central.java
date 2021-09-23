package com.example.loravisual.blecommunication;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.example.loravisual.R;
import com.example.loravisual.screens.ExploreScreens.ConfigSetter;

import java.util.Arrays;
import java.util.UUID;

import static android.bluetooth.BluetoothDevice.TRANSPORT_LE;

/**
 * this class defines the 'BLE Central' role of the app.
 */
public class Central {
    private static final String TAG = "at Central";

    private BluetoothGatt bluetoothGatt;

    Context context;
    Scanner scanner;

    // several UUIDs to distinguish different service characteristics.
    private UUID serviceUUID;
    private UUID notificationCharacteristic_UUID;
    private UUID changeConfigCharacteristic_UUID;
    private UUID sendCommandCharacteristic_UUID;
    private UUID loopCommandCharacteristic_UUID;
    private UUID prepareSenderCharacteristic_UUID;
    private UUID descriptor_UUID;

    private BluetoothDevice bluetoothDevice;
    private int device;
    private int mode;

    boolean reopenConnection;

    /**
     * constructor assigns UUIDs depending on which LoRa device it is supposed to connect to.
     * @param context
     * @param scanner
     * @param device LoRa receiver or sender
     * @param mode scenario exploration or experiment
     */
    public Central(Context context, Scanner scanner, int device, int mode) {
        this.context = context;
        this.scanner = scanner;
        this.device = device;
        this.mode = mode;
        reopenConnection = true;        // used to customize the reconnecting behaviour

        serviceUUID = device == BLEHandler.SENDER ? UUID.fromString(context.getResources().getString(R.string.Sender_Service_UUID))
                : UUID.fromString(context.getResources().getString(R.string.Receiver_Service_UUID));

        notificationCharacteristic_UUID = device == BLEHandler.SENDER ? UUID.fromString(context.getResources().getString(R.string.Notification_Sender_Characteristic_UUID))
                : UUID.fromString(context.getResources().getString(R.string.Notification_Receiver_Characteristic_UUID));

        changeConfigCharacteristic_UUID = device == BLEHandler.SENDER ? UUID.fromString(context.getResources().getString(R.string.Config_Change_Sender_Characteristic_UUID))
                : UUID.fromString(context.getResources().getString(R.string.Config_Change_Receive_Characteristic_UUID));

        sendCommandCharacteristic_UUID = device == BLEHandler.SENDER ? UUID.fromString(context.getResources().getString(R.string.Send_Command_Characteristic_UUID))
                : UUID.fromString(context.getResources().getString(R.string.Experiment_Settings_To_Receiver_Characteristic_UUID));       // for a receiver this UUID is used to send experiment settings

        if (device == BLEHandler.SENDER) {
            loopCommandCharacteristic_UUID = UUID.fromString(context.getResources().getString(R.string.Loop_Command_Characteristic_UUID));
            prepareSenderCharacteristic_UUID = UUID.fromString(context.getResources().getString(R.string.Prepare_Sender_Characteristic_UUID));
        }


        descriptor_UUID = UUID.fromString(context.getResources().getString(R.string.Descriptor_UUID));
    }

    /**
     * this callback handles behaviour on various state changes
     */
    private BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        @Override
        /**
         * triggered when a device connects or disconnects. Automatically reconnects on disconnection,
         * unless specified differently (controlled by boolean reopenConnection)
         */
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.i(TAG, "connection state changed.");
            super.onConnectionStateChange(gatt, status, newState);

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "connection established.");
                gatt.discoverServices();
                sendConnStateBroadcast(true);
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "disconnected.");
                sendConnStateBroadcast(false);
                gatt.close();
                if(reopenConnection)
                    scanner.startScanning();
            }
        }

        @Override
        /**
         * triggered when a service is discovered after a connection was established.
         * LoRa sender and receiver define one service each.
         */
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            Log.i(TAG, "status at discovered " + status);

            BluetoothGattCharacteristic characteristic = gatt.getService(serviceUUID).getCharacteristic(notificationCharacteristic_UUID);
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(descriptor_UUID);

            gatt.readDescriptor(descriptor);

            // usually we want the LoRa device to set the configuration specified in the spinners immediately on connection,
            // this ensures that even after a disconnect and subsequent reconnect the parameters, specified in the spinners
            // are consistent with the ones set at the LoRa devices. Only in case of experiment mode + LoRa sender device we
            // want to wait until a user presses 'prepare sender'.
            if(mode == BLEHandler.EXPERIMENT && device == BLEHandler.SENDER)
                return;

            // the command to either set the parameters given in the spinners, or to prepare the experiment at the receiver side,
            // needs to be sent with a small delay. Otherwise it will get lost.
            final Handler handler = new Handler(Looper.getMainLooper());
            final Runnable runnable = new Runnable() {
                public void run() {
                    Log.i(TAG, "arrived at execution of handler...");
                    if (mode == BLEHandler.EXPLORE) {
                        writeChangeConfigCharacteristic(ConfigSetter.config);
                    } else {
                        if(device == BLEHandler.RECEIVER)           // in experiment mode: the SENDER is prepared from a screen button (not automatically on connection like the receiver)
                            writeExperimentSettingsCharacteristic(ConfigSetter.config);
                    }
                }
            };
            handler.postDelayed(runnable, 400);
        }

        @Override
        /**
         * sets the descriptor of the notification characteristic (for data flow from LoRa devices to app) to enabled, when triggered.
         */
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorRead(gatt, descriptor, status);
            gatt.setCharacteristicNotification(descriptor.getCharacteristic(), true);

            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            gatt.writeDescriptor(descriptor);
        }

        @Override
        /**
         * triggered when the LoRa device forwards information. Handles behaviour in that case.
         * It mostly notifies the relevant screen via app intern broadcast.
         */
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            byte[] bytes = characteristic.getValue();
            if (bytes[0] == -2) {        // notification that the LoRa configuration was successfully changed (in either receiver or sender explore mode) or that sender is prepared or that receiver is prepared (in experiment)
                sendGeneralBroadcast(-2);
                return;
            }

            if(bytes[0] == -3 || bytes[0] == -4 || bytes[0] == -5 || bytes[0] == -6) {  // see general broadcast receiver implementations in the screen classes for meaning of the numbers
                sendGeneralBroadcast(bytes[0]);
                return;
            }

            if (device == BLEHandler.RECEIVER) {
                if (bytes[0] == -1) {       // RSSI/SNR notification in explore mode
                    for (int i = 0; i < bytes.length; i++) {
                        Log.i(TAG, "value received from connected device: " + "byte: " + i + ": " + bytes[i]);
                    }
                    sendRSSIsNRBroadcast(bytes);
                } else {        // message notification in explore mode
                    bytes = filterContent(bytes);
                    Log.i(TAG, "value received from connected device: " + new String(bytes));
                    sendMsgBroadcast(bytes);
                }
            }
        }
    };

    /**
     * we delimit all LoRa transmissions with '.', this method helps us find the end in a 20 bytes long array (length of BLE packet)
     * @param msg
     * @return
     */
    private byte[] filterContent(byte[] msg) {
        int index = 0;
        for (int i = 0; i < msg.length; i++) {
            if (msg[i] == 46) {
                index = i;      // LAST (otherwise problems with the message number 46) occurrence of 46 / '.' is the delimiter
            }
        }
        return Arrays.copyOfRange(msg, 0, index);
    }

    /**
     * the following five methods write a command and corresponding data to the appropriate characteristic.
     * The data type of the argument varies for practical reasons. (The transmitted content is always a byte array in the end)
     * @param configBytes
     */
    public void writeChangeConfigCharacteristic(byte[] configBytes) {
        BluetoothGattCharacteristic characteristic = bluetoothGatt.getService(serviceUUID).getCharacteristic(changeConfigCharacteristic_UUID);
        characteristic.setValue(configBytes);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void writeSendCommandCharacteristic(String loraMsg) {
        BluetoothGattCharacteristic characteristic = bluetoothGatt.getService(serviceUUID).getCharacteristic(sendCommandCharacteristic_UUID);
        byte[] bytes = loraMsg.getBytes();
        characteristic.setValue(bytes);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void writeLoopCommandCharacteristic(String s) {
        BluetoothGattCharacteristic characteristic = bluetoothGatt.getService(serviceUUID).getCharacteristic(loopCommandCharacteristic_UUID);
        byte[] bytes = s.getBytes();
        characteristic.setValue(bytes);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void writePrepareSenderCharacteristic(byte[] bytes) {
        BluetoothGattCharacteristic characteristic = bluetoothGatt.getService(serviceUUID).getCharacteristic(prepareSenderCharacteristic_UUID);
        characteristic.setValue(bytes);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void writeExperimentSettingsCharacteristic(byte[] bytes) {
        BluetoothGattCharacteristic characteristic = bluetoothGatt.getService(serviceUUID).getCharacteristic(sendCommandCharacteristic_UUID);
        characteristic.setValue(bytes);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void connect(BluetoothDevice bluetoothDevice) {
        this.bluetoothDevice = bluetoothDevice;
        bluetoothGatt = bluetoothDevice.connectGatt(context, false, gattCallback, TRANSPORT_LE);
    }

    /**
     * If we 'manually' close the BLE connection, we don't want it to reopen automatically:
     */
    public void closeConnection() {
        if (bluetoothGatt != null) {
            reopenConnection = false;
            bluetoothGatt.close();
        }
    }

    /**
     * the following four methods notify the currently active screen about various events.
     * (we cannot achieve the screen behaviour in case of these events by reference in this class)
     * @param connected
     */
    private void sendConnStateBroadcast(boolean connected) {        // for sender and receiver
        Intent intent = new Intent("com.example.loravisual.CONNECTION_STATE_CHANGED");
        intent.putExtra("com.example.loravisual.CONNECTION_STATE", connected);
        context.sendBroadcast(intent);
    }

    private void sendRSSIsNRBroadcast(byte[] values) {              // for receiver
        Intent intent = new Intent("com.example.loravisual.RSSI_RECEIVED");
        intent.putExtra("com.example.loravisual.RSSISNR", values);
        context.sendBroadcast(intent);
    }

    private void sendMsgBroadcast(byte[] msg) {                     // for receiver
        Intent intent = new Intent("com.example.loravisual.MSG_RECEIVED");
        intent.putExtra("com.example.loravisual.MSG", msg);
        context.sendBroadcast(intent);
    }

    private void sendGeneralBroadcast(int status) {                 // for sender and receiver
        Intent intent = new Intent("com.example.loravisual.GENERAL_BROADCAST");
        intent.putExtra("com.example.loravisual.GENERAL", status);
        context.sendBroadcast(intent);
    }
}
