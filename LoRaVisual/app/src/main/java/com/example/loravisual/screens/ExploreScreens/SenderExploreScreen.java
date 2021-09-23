package com.example.loravisual.screens.ExploreScreens;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.example.loravisual.R;
import com.example.loravisual.blecommunication.BLEHandler;
import com.example.loravisual.lvutilities.GUICountDownTimer;
import com.example.loravisual.lvutilities.LocationHandler;

/**
 * screen for controlling the sending board.
 */
public class SenderExploreScreen extends AppCompatActivity {
    private static final String TAG = "at SenderExploreScreen";

    private BLEHandler bleHandler;
    private LocationHandler locationHandler;

    private BroadcastReceiver connStateBroadcastReceiver;
    private BroadcastReceiver generalBroadcastReceiver;

    private EditText loraMsgEdit;

    private boolean connected;
    private boolean ready;
    private boolean pingDisconnect;

    Context context = this;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sender_explore_screen);

        getSupportActionBar().setTitle("Control Sender");
        getSupportActionBar().setSubtitle(getString(R.string.disconnected));

        ConfigSetter.setExploreSpinners(this);

        setupCentral();

        loraMsgEdit = findViewById(R.id.edit_msg);

        setupConnStateBroadcastReceiver();
        setupGeneralBroadcastReceiver();

        locationHandler = new LocationHandler(this, findViewById(R.id.latitude),
                findViewById(R.id.longitude), findViewById(R.id.altitude), findViewById(R.id.distance), null, null);

        setLongClickListenerSendPing();
    }

    /**
     * connects phone to the sending board.
     */
    private void setupCentral() {
        bleHandler = new BLEHandler(this, BLEHandler.SENDER, BLEHandler.EXPLORE);
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
     * sends a command via ble, to send the message defined in the corresponding EditText via LoRa.
     * @param view
     */
    public void transmitSendCommand(View view) {
        String msg = loraMsgEdit.getText().toString();
        sendOrPing(msg);
    }

    /**
     * the same button that activates the method above, will initiate a ping when long clicked.
     */
    private void setLongClickListenerSendPing() {
        findViewById(R.id.confirm_msg).setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                Toast.makeText(context, "pinged receiver", Toast.LENGTH_SHORT).show();
                pingDisconnect = true;
                String msg = loraMsgEdit.getText().toString();
                sendOrPing("&" + msg);

                return true;
            }
        });
    }

    /**
     * convenience method for ensuring proper usage of GUI elements for sending(for example only send command via ble, when connected to board).
     * @param msg content that is supposed to be sent via LoRa
     */
    private void sendOrPing(String msg) {
        if (ready) {
            if (msg.length() > 18) {
                Toast.makeText(this, "maximum 18 characters", Toast.LENGTH_SHORT).show();
                return;
            }
            bleHandler.writeSendCommandCharacteristic(msg);
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * instruct board to send LoRa messages in a loop (lets user specify number of and time between messages via Alert Dialogs.
     * @param view
     */
    public void transmitSendLoopCommand(View view) {
        if (ready) {
            if (hasSpecialCharacters()) {
                Toast.makeText(this, "message field may not start with '!', '#' or '$' for loops", Toast.LENGTH_SHORT).show();
                return;
            }
            final EditText numberOfMessagesEdit = new EditText(this);
            makeAlertDialog("Number of messages", "Set the number of LoRa transmissions", numberOfMessagesEdit, true, "!");
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * cancel an already initiated sending loop, adjusts GUI accordingly.
     * @param view
     */
    public void cancelSendLoopCommand(View view) {
        adjustGUIToLooping(false);
        bleHandler.writeLoopCommandCharacteristic("$");
    }

    /**
     * tag current location. will then let the user see the distance to this location when moving away.
     * @param view
     */
    public void tagLocation(View view) {
        locationHandler.tagLocation();
    }

    /**
     * ensures proper usage (some characters have special usage on boards.
     * @return
     */
    private boolean hasSpecialCharacters() {
        String s = loraMsgEdit.getText().toString();
        return s.startsWith("!") || s.startsWith("#") || s.startsWith("$");
    }

    /**
     * sets up a broadcast receiver for notifications on the ble connection status.
     */
    private void setupConnStateBroadcastReceiver() {
        connStateBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.CONNECTION_STATE_CHANGED".equals(intent.getAction())) {
                    connected = intent.getBooleanExtra("com.example.loravisual.CONNECTION_STATE", false);
                    if (connected) {
                        if(!pingDisconnect)
                            Toast.makeText(context, "connected to device", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                    } else {
                        if(!pingDisconnect)
                            Toast.makeText(context, "disconnected", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.disconnected));
                        ready = false;
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.CONNECTION_STATE_CHANGED");
        registerReceiver(connStateBroadcastReceiver, filter);
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
                        if(!pingDisconnect)
                            Toast.makeText(context, "LoRa parameters consistent", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                        ready = true;
                        return;
                    } else if (status == -3) {
                        Toast.makeText(context, "correct response received", Toast.LENGTH_SHORT).show();
                    } else if (status == -4) {
                        Toast.makeText(context, "response received but did not match", Toast.LENGTH_SHORT).show();
                    } else if (status == -5) {
                        Toast.makeText(context, "no response received", Toast.LENGTH_SHORT).show();
                    }
                    pingDisconnect = false;
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.GENERAL_BROADCAST");
        registerReceiver(generalBroadcastReceiver, filter);
    }

    int loopSeconds = 0;

    /**
     * creates the AlertDialog sequence for specifying looping parameters
     *
     * @param title
     * @param message
     * @param editText
     * @param recursive
     * @return
     */
    private void makeAlertDialog(String title, String message, final EditText editText, boolean recursive, String prefix) {
        final EditText timeBetweenMessagesEdit = new EditText(this);

        new AlertDialog.Builder(this)
                .setTitle(title)
                .setMessage(message)
                .setCancelable(true)
                .setView(editText)
                .setPositiveButton("confirm", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        String num = editText.getText().toString();
                        if(recursive)
                        Toast.makeText(context, "Device will send " + num + " messages", Toast.LENGTH_SHORT).show();
                        else
                            Toast.makeText(context, "Time between transmissions is " + num + " seconds", Toast.LENGTH_SHORT).show();
                        String command = prefix + num;
                        bleHandler.writeLoopCommandCharacteristic(command);

                        if (recursive) {
                            loopSeconds += (Integer.parseInt(num) -1);
                            makeAlertDialog("Time between messages", "Set the time between LoRa transmissions", timeBetweenMessagesEdit, false, "#");
                        } else {
                            loopSeconds *= Integer.parseInt(num);
                            adjustGUIToLooping(true);
                            final Handler handler = new Handler(Looper.getMainLooper());
                            final Runnable runnable = new Runnable() {
                                public void run() {
                                    bleHandler.writeLoopCommandCharacteristic(loraMsgEdit.getText().toString());
                                    startTimers(loopSeconds, findViewById(R.id.htext));
                                }
                            };
                            handler.postDelayed(runnable, 200);
                        }
                    }
                }).show();
    }

    GUICountDownTimer timer;

    /**
     * starts a timer on the GUI telling the user how long the device will keep looping.
     * @param seconds
     * @param timeView
     */
    private void startTimers(int seconds, TextView timeView) {
        int hoursD = seconds / 3600;
        int remainder = seconds % 3600;
        int minutesD = remainder / 60;
        int secondsD = remainder % 60;
        if(timer != null)
            timer.cancel();

        timer = (GUICountDownTimer) new GUICountDownTimer(seconds * 1000, 1000, hoursD, minutesD, secondsD, timeView) {

            public void onFinish() {
                timeView.setText("00 : 00 : 00");
                adjustGUIToLooping(false);
            }

        }.start();
    }

    /**
     * sets some GUI elements according to the current loop status.
     * @param looping
     */
    private void adjustGUIToLooping(boolean looping) {
        if(looping) {
            findViewById(R.id.loop_msg).setVisibility(View.GONE);
            findViewById(R.id.cancel_loop_msg).setVisibility(View.VISIBLE);
            findViewById(R.id.loop_countdown_layout).setVisibility(View.VISIBLE);
            findViewById(R.id.confirm_msg).setBackgroundColor(Color.parseColor("#ffc69f"));
            findViewById(R.id.change_config_button).setBackgroundColor(Color.parseColor("#66769d"));
        } else {
            findViewById(R.id.loop_msg).setVisibility(View.VISIBLE);
            findViewById(R.id.cancel_loop_msg).setVisibility(View.GONE);
            findViewById(R.id.loop_countdown_layout).setVisibility(View.GONE);
            findViewById(R.id.confirm_msg).setBackgroundColor(Color.parseColor("#eea47f"));
            findViewById(R.id.change_config_button).setBackgroundColor(Color.parseColor("#00539C"));
            loopSeconds = 0;
        }
        findViewById(R.id.change_config_button).setEnabled(!looping);
        findViewById(R.id.confirm_msg).setEnabled(!looping);
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