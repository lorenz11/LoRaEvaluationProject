package com.example.loravisual.blecommunication;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

import com.example.loravisual.R;

import java.util.UUID;

/**
 * This class provides functionality for scanning for BLE advertisements with the appropriate UUID
 */
public class Scanner {
    private static final String TAG = "at Scanner";
    Context context;
    BluetoothAdapter bluetoothAdapter;
    BluetoothLeScanner bluetoothLeScanner;

    private Central central;

    private UUID serviceUUID;

    private int device;
    private int mode;

    /**
     * constructor sets the appropriate UUID to scan for. This UUID enables
     * the app to differentiate between a sender device advertisement, a
     * receiver device advertisement and other random advertisements
     * @param context
     * @param bluetoothAdapter
     * @param device, the app is supposed to connect to (sender or receiver)
     * @param mode
     */
    public Scanner(Context context, BluetoothAdapter bluetoothAdapter, int device, int mode) {
        this.context = context;
        this.bluetoothAdapter = bluetoothAdapter;
        this.device = device;
        this.mode = mode;

        serviceUUID = device == BLEHandler.SENDER ? UUID.fromString(context.getResources().getString(R.string.Sender_Service_UUID))
                :UUID.fromString(context.getResources().getString(R.string.Receiver_Service_UUID));
    }

    /**
     * this callback is triggered when the device detects an advertisement.
     * It initiates the connection if the advertisement is associated with
     * the correct UUID.
     */
    private final ScanCallback scanCallback = new ScanCallback() {

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (result.getScanRecord().getServiceUuids() != null) {
                UUID resultUUID = result.getScanRecord().getServiceUuids().get(0).getUuid();
                Log.i(TAG, "result UUID: " + resultUUID);
                if (resultUUID.equals(serviceUUID)) {
                    Log.i(TAG, "detected device.");
                    central.connect(result.getDevice());
                    stopScanning();
                }
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.i(TAG, "scan failed.");
        }
    };

    /**
     * starts the scan.
     */
    public void startScanning() {
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        central = new Central(context, this, device, mode);
        Log.i(TAG, "started scanning.");
        bluetoothLeScanner.startScan(scanCallback);
    }

    /**
     * stops the scan.
     */
    public void stopScanning() {
        if (bluetoothLeScanner != null) {
            Log.i(TAG, "scanning stopped.");
            bluetoothLeScanner.stopScan(scanCallback);
            bluetoothLeScanner = null;
        }
    }

    /**
     * We need to close a connection we don't use anymore to avoid data leaks.
     */
    public void closeConnection() {
        if(central != null) {
            central.closeConnection();
            central = null;
        }
    }

    /**
     * returns a reference to the central object.
     * @return
     */
    public Central getCentral() {
        return central;
    }
}
