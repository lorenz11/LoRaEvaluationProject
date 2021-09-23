package com.example.loravisual.screens.ExperimentScreens;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.example.loravisual.R;
import com.example.loravisual.blecommunication.BLEHandler;
import com.example.loravisual.database.DatabaseHandler;
import com.example.loravisual.lvutilities.AccelerometerHandler;
import com.example.loravisual.lvutilities.LocationHandler;
import com.example.loravisual.screens.ExploreScreens.ConfigSetter;

/**
 * This screen allows a user to prepare the LoRa sender for an experiment and log data about it.
 */
public class PrepareSenderScreen extends AppCompatActivity {
    private static final String TAG = "at PrepareSenderScreen";

    private BLEHandler bleHandler;
    private DatabaseHandler databaseHandler;
    private LocationHandler locationHandler;
    private AccelerometerHandler accelerometerHandler;

    private BroadcastReceiver connStateBroadcastReceiver;
    private BroadcastReceiver generalBroadcastReceiver;

    private Button prepareSenderButton;

    int experimentNumber;

    private boolean connected;
    private boolean ready;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_prepare_sender_screen);
        prepareSenderButton = findViewById(R.id.prep_sender_button);

        experimentNumber = getIntent().getIntExtra("absolutePos", -1);

        getSupportActionBar().setTitle("Experiment " + experimentNumber);
        getSupportActionBar().setSubtitle(getString(R.string.disconnected));

        ConfigSetter.setExploreSpinners(this);
        setupCentral();
        databaseHandler = new DatabaseHandler(this);

        setupConnStateBroadcastReceiver();
        setupGeneralBroadcastReceiver();

        locationHandler = new LocationHandler(this, findViewById(R.id.latitude),
                findViewById(R.id.longitude), findViewById(R.id.altitude), null, null, null);
        accelerometerHandler = new AccelerometerHandler(this, findViewById(R.id.accelerate_indicator));
    }

    /**
     * connects the smartphone to the LoRa sender on screen creation.
     */
    private void setupCentral() {
        bleHandler = new BLEHandler(this, BLEHandler.SENDER, BLEHandler.EXPERIMENT);
        bleHandler.connect();
    }

    /**
     * sends the command to change the LoRa parameter configuration to the values in the spinners
     * for pre- experiment communication. Also logs current GPS data.
     * @param view
     */
    public void prepareSender(View view) {
        if (ready) {
            bleHandler.writePrepareSenderCharacteristic(ConfigSetter.config);
            writeGPSDataToDB(locationHandler.getLatitude(), locationHandler.getLongitude(), locationHandler.getAltitude());
            Toast.makeText(this, "senders GPS data logged", Toast.LENGTH_SHORT).show();
            isPrepared = true;

            prepareSenderButton.setVisibility(View.GONE);
            findViewById(R.id.cancel_prep_button).setVisibility(View.VISIBLE);
            ConfigSetter.enableSpinners(false);
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
        }
    }

    private boolean isPrepared;
    /**
     * starts the @StartExperimentScreen, logs dummy GPS data if no real data available.
     * logs the smartphones orientation.
     * @param view
     */
    public void toStartExperiment(View view) {
        if(!isPrepared)
            writeGPSDataToDB("-1", "-1", "-1");
        String senderOrientation = ((TextView) findViewById(R.id.accelerate_indicator)).getText().toString();
        databaseHandler.setValueInExpInfo(senderOrientation, "sender_orientation", experimentNumber);
        accelerometerHandler.close();
        Intent intent = new Intent(this, StartExperimentScreen.class);
        intent.putExtra("absolutePos", experimentNumber);
        intent.putExtra("orientation", senderOrientation);
        startActivity(intent);
    }

    /**
     * for decomposition.
     * @param lat
     * @param lon
     * @param alt
     */
    private void writeGPSDataToDB(String lat, String lon, String alt) {
        databaseHandler.setValueInExpInfo(lat, "sender_latitude", experimentNumber);
        databaseHandler.setValueInExpInfo(lon, "sender_longitude", experimentNumber);
        databaseHandler.setValueInExpInfo(alt, "sender_altitude", experimentNumber);
    }

    /**
     * tells the LoRa sender to cancel the thread for a new experiment and adjusts the GUI.
     * @param view
     */
    public void cancelPreparation(View view) {
        if (ready) {
            isPrepared = false;
            bleHandler.writePrepareSenderCharacteristic(new byte[]{-1});

            prepareSenderButton.setVisibility(View.VISIBLE);
            findViewById(R.id.cancel_prep_button).setVisibility(View.GONE);
            ConfigSetter.enableSpinners(true);
            Toast.makeText(this, "sender preparation canceled", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * sets up a broadcast receiver for notifications on the ble connection status.
     */
    private void setupConnStateBroadcastReceiver() {
        connStateBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.CONNECTION_STATE_CHANGED".equals(intent.getAction())) {
                    final Handler handler = new Handler(Looper.getMainLooper());
                    final Runnable runnable = new Runnable() {
                        public void run() {
                            connected = intent.getBooleanExtra("com.example.loravisual.CONNECTION_STATE", false);
                            if (connected) {
                                Toast.makeText(context, "connected to device", Toast.LENGTH_SHORT).show();
                                getSupportActionBar().setSubtitle(getString(R.string.connected));
                                ready = true;
                            } else {
                                Toast.makeText(context, "disconnected", Toast.LENGTH_SHORT).show();
                                getSupportActionBar().setSubtitle(getString(R.string.disconnected));
                                ready = false;
                            }
                        }
                    };
                    handler.postDelayed(runnable, 2000);         // waiting time equivalent to the time in onServicesDiscovered for other BLE connections
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.CONNECTION_STATE_CHANGED");
        registerReceiver(connStateBroadcastReceiver, filter);
    }

    /**
     * for general notifications, displays appropriate toast.
     */
    private void setupGeneralBroadcastReceiver() {
        generalBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.GENERAL_BROADCAST".equals(intent.getAction())) {
                    int status = intent.getIntExtra("com.example.loravisual.GENERAL", 0);
                    if (status == -2) {
                        Toast.makeText(context, "Sender device ready", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
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
        locationHandler.close();
        unregisterReceiver(connStateBroadcastReceiver);
        unregisterReceiver(generalBroadcastReceiver);

        if (connected)
            Toast.makeText(this, "disconnected", Toast.LENGTH_SHORT).show();
    }
}