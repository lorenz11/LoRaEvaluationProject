package com.example.loravisual.screens.ExploreScreens;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.loravisual.R;
import com.example.loravisual.blecommunication.BLEHandler;
import com.example.loravisual.lvutilities.LVUtil;
import com.example.loravisual.lvutilities.LocationHandler;

import java.util.Timer;
import java.util.TimerTask;

/**
 * screen for controlling the receiving board.
 */
public class ReceiverExploreScreen extends AppCompatActivity {
    private static final String TAG = "at ReceiverExploreScreen";

    private BLEHandler bleHandler;
    private LocationHandler locationHandler;

    private BroadcastReceiver broadcastReceiverConnected;
    private BroadcastReceiver broadcastReceiverRSSI;
    private BroadcastReceiver broadcastReceiverLoraMsg;
    private BroadcastReceiver generalBroadcastReceiver;

    private TextView msgTextView;
    private TextView rssiTextView;
    private TextView snrTextView;
    private TextView timeTextView;

    private boolean connected;
    private boolean ready;

    boolean colorToggle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_receiver_explore_screen);

        getSupportActionBar().setTitle("Control Receiver");
        getSupportActionBar().setSubtitle(getString(R.string.disconnected));

        ConfigSetter.setExploreSpinners(this);
        setupCentral();

        msgTextView = findViewById(R.id.msg_textview);
        rssiTextView = findViewById(R.id.rssi_textview);
        snrTextView = findViewById(R.id.snr_textview);
        timeTextView = findViewById(R.id.time_textview);

        setupConnStateBroadcastReceiver();
        setupRSSIsNRBroadcastReceiver();
        setupMsgBroadcastReceiver();
        setupGeneralBroadcastReceiver();

        locationHandler = new LocationHandler(this, findViewById(R.id.latitude),
                findViewById(R.id.longitude), findViewById(R.id.altitude), findViewById(R.id.distance), null, null);
    }

    /**
     * connects phone to the receiving board.
     */
    public void setupCentral() {
        bleHandler = new BLEHandler(this, BLEHandler.RECEIVER, BLEHandler.EXPLORE);
        bleHandler.connect();
    }

    /**
     * sends the command to change the LoRa configuration according to selected spinner values via ble.
     * @param view
     */
    public void changeConfig(View view) {
        if (ready) {
            bleHandler.writeChangeConfigCharacteristic(ConfigSetter.config);
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * tag current location. will then let the user see the distance to this location when moving away.
     * @param view
     */
    public void tagLocation(View view) {
        locationHandler.tagLocation();
    }

    /**
     * sets up a broadcast receiver for notifications on the ble connection status.
     */
    private void setupConnStateBroadcastReceiver() {
        broadcastReceiverConnected = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.CONNECTION_STATE_CHANGED".equals(intent.getAction())) {
                    connected = intent.getBooleanExtra("com.example.loravisual.CONNECTION_STATE", false);
                    if (connected) {
                        Toast.makeText(context, "connected to device", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                    } else {
                        Toast.makeText(context, "disconnected", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.disconnected));
                        ready = false;
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.CONNECTION_STATE_CHANGED");
        registerReceiver(broadcastReceiverConnected, filter);

    }

    /**
     * sets up a broadcast receiver for notifications about the rssi and snr values determined at the receiver board.
     */
    private void setupRSSIsNRBroadcastReceiver() {
        broadcastReceiverRSSI = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.RSSI_RECEIVED".equals(intent.getAction())) {
                    byte[] bytes = intent.getByteArrayExtra("com.example.loravisual.RSSISNR");
                    rssiTextView.setText("RSSI: -" + (LVUtil.makeByteUnsigned(bytes[1])));
                    snrTextView.setText("SNR: " + (bytes[2]));
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.RSSI_RECEIVED");
        registerReceiver(broadcastReceiverRSSI, filter);

    }

    int[] units; // seconds, minutes, hours, ticks
    String time = "00 : 00 : 00";
    Timer timer;

    /**
     * sets up a broadcast receiver for notifications on received LoRa messages.
     * starts a timer, when receiving something (to know when the next message should arrive
     * in case the sending board sends in a loop.
     */
    private void setupMsgBroadcastReceiver() {
        broadcastReceiverLoraMsg = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
            if ("com.example.loravisual.MSG_RECEIVED".equals(intent.getAction())) {
                String msg = new String(intent.getByteArrayExtra("com.example.loravisual.MSG"));
                colorToggle = !colorToggle;
                if (colorToggle) {
                    msgTextView.setTextColor(getResources().getColor(R.color.colorPrimaryDark, null));
                } else {
                    msgTextView.setTextColor(getResources().getColor(R.color.white, null));
                }
                msgTextView.setText(msg);
                timeTextView.setText(R.string.timing_default_string);

                if (timer != null)
                    timer.cancel();
                units = new int[]{0, 0, 0, 0};
                timer = new Timer();
                timer.scheduleAtFixedRate(new TimerTask() {
                    @Override
                    public void run() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                manageTimer();
                            }
                        });
                    }
                }, 1000, 1000);

            }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.MSG_RECEIVED");
        registerReceiver(broadcastReceiverLoraMsg, filter);
    }

    /**
     * manages the timing, used in method above.
     */
    private void manageTimer() {
        units[3]++;                 // ticks
        units[0] = units[3] % 60;   // seconds
        int remainder = units[3] % 3600;
        units[1] = remainder / 60;   // minutes
        units[2] = units[3] / 3600; // hours

        time = units[2] < 10 ? "0" + units[2] + " : " : units[2] + " : ";
        time += units[1] < 10 ? "0" + units[1] + " : " : units[1] + " : ";
        time += units[0] < 10 ? "0" + units[0] : units[0];
        timeTextView.setText(time);
    }

    /**
     * sets up a broadcast receiver for other BLE notifications.
     */
    private void setupGeneralBroadcastReceiver() {
        generalBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.GENERAL_BROADCAST".equals(intent.getAction())) {
                    int status = intent.getIntExtra("com.example.loravisual.GENERAL", 0);
                    if (status == -2) {
                        Toast.makeText(context, "LoRa parameters consistent", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                        ready = true;
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.GENERAL_BROADCAST");
        registerReceiver(generalBroadcastReceiver, filter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bleHandler.closeConnection();
        if(timer!=null)
            timer.cancel();

        locationHandler.close();
        unregisterReceiver(broadcastReceiverConnected);
        unregisterReceiver(broadcastReceiverRSSI);
        unregisterReceiver(broadcastReceiverLoraMsg);
        unregisterReceiver(generalBroadcastReceiver);

        if (connected)
            Toast.makeText(this, "disconnected", Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onResume() {
        super.onResume();
        timeTextView.setText(time);
    }
}