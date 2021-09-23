package com.example.loravisual.screens.ExperimentScreens;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.Location;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.example.loravisual.R;
import com.example.loravisual.async.ClientThread;
import com.example.loravisual.blecommunication.BLEHandler;
import com.example.loravisual.database.DatabaseHandler;
import com.example.loravisual.lvutilities.AccelerometerHandler;
import com.example.loravisual.lvutilities.GUICountDownTimer;
import com.example.loravisual.lvutilities.LVUtil;
import com.example.loravisual.lvutilities.LocationHandler;
import com.example.loravisual.screens.ExploreScreens.ConfigSetter;

import java.io.IOException;
import java.util.ArrayList;

public class StartExperimentScreen extends AppCompatActivity {
    private static final String TAG = "at StartExperimentScreen";

    DatabaseHandler databaseHandler = new DatabaseHandler(this);
    private BLEHandler bleHandler;
    private LocationHandler locationHandler;
    private AccelerometerHandler accelerometerHandler;

    private BroadcastReceiver connStateBroadcastReceiver;
    private BroadcastReceiver broadcastReceiverRSSI;
    private BroadcastReceiver broadcastReceiverLoraMsg;
    private BroadcastReceiver generalBroadcastReceiver;

    int[] multipliers = new int[7];
    int[] arrayResourceIds;

    private ArrayList<String> entry;

    private TextView msgTextView;
    private TextView rssiSnrTextView;

    private EditText hoursEdit;
    private EditText minutesEdit;
    private EditText secondsEdit;

    int experimentNumber;

    int hoursD = 0;
    int minutesD = 0;
    int secondsD = 0;

    int duration;
    int iterations;

    private boolean connected;
    private boolean ready;
    private boolean colorToggle;
    private boolean justShowingInfo;
    private boolean connToastsDeactivated;

    Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_start_experiment_screen);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        context = this;

        experimentNumber = getIntent().getIntExtra("absolutePos", -1);
        ((TextView)findViewById(R.id.accelerate_indicator_at_sender)).setText(getIntent().getStringExtra("orientation") + " (at sender)");
        getSupportActionBar().setTitle("Experiment " + experimentNumber);

        initSummary();

        if (databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.STATE).equals(DatabaseHandler.READY)) {
            getSupportActionBar().setSubtitle(getString(R.string.disconnected));
            setupCentral();

            msgTextView = findViewById(R.id.msg_content);
            rssiSnrTextView = findViewById(R.id.rssi_snr_text);

            setupConnStateBroadcastReceiver();
            setupMsgBroadcastReceiver();
            setupRSSIsNRBroadcastReceiver();
            setupGeneralBroadcastReceiver();

            setupLocationHandler();
            accelerometerHandler = new AccelerometerHandler(this, findViewById(R.id.accelerate_indicator_rec));
        } else {
            setupFinishedGUI();
            justShowingInfo = true;
        }
    }

    /**
     * triggered when pressing the "start" button. Starts the delay timer with specified delay.
     * Sends the settings byte array to the board. (in case the user presses "yes" in the alert dialog)
     * @param view
     */
    public void makeConfirmationDialog(View view) {
        new AlertDialog.Builder(this)
                .setTitle("Are you sure?")
                .setCancelable(false)
                .setPositiveButton("yes", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        startExperiment();
                    }
                })
                .setNegativeButton("no", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        return;
                    }
                }).show();
    }

    long total;

    private void startExperiment() {
        findViewById(R.id.countdownpickerlayout).setVisibility(View.GONE);
        findViewById(R.id.start_button).setVisibility(View.GONE);
        findViewById(R.id.ping_button).setVisibility(View.GONE);
        findViewById(R.id.delaylayout).setVisibility(View.VISIBLE);

        hoursD = Integer.parseInt(hoursEdit.getText().toString());
        minutesD = Integer.parseInt(minutesEdit.getText().toString());
        secondsD = Integer.parseInt(secondsEdit.getText().toString());
        if (minutesD > 59 || secondsD > 59 || minutesD < 0 || secondsD < 0 || hoursD < 0) {
            Toast.makeText(this, "Number of seconds and minutes must be smaller than 60 and positive", Toast.LENGTH_LONG).show();
            return;
        }
        total = (hoursD * 3600 + minutesD * 60 + secondsD) * 1000;

        byte[] expSettings;
        if(ConfigSetter.config[4] < 4) {                        // see strings resource file for assignment of power values to ConfigSetter.config
            expSettings = createSettingsBytes(total + 17000);
        } else {
            expSettings = createSettingsBytes(total);
        }

        if (ready) {
            bleHandler.writeExperimentSettingsCharacteristic(expSettings);
            String remainingTime = hoursD < 10 ? "0" + hoursD + " : " : Integer.toString(hoursD) + " : ";
            remainingTime += minutesD < 10 ? "0" + minutesD + " : " : Integer.toString(minutesD) + " : ";
            remainingTime += secondsD < 10 ? "0" + secondsD : Integer.toString(secondsD);
            Toast.makeText(this, "waiting for ACK", Toast.LENGTH_SHORT).show();
            ((TextView) findViewById(R.id.htext)).setText(remainingTime);
            connToastsDeactivated = true;
        } else {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
            return;
        }
    }

    GUICountDownTimer timer;

    private void startTimers(long total, TextView timeView, boolean first) {
        if (timer != null)
            timer.cancel();

        timer = (GUICountDownTimer) new GUICountDownTimer(total, 1000, hoursD, minutesD, secondsD, timeView) {

            public void onFinish() {
                timeView.setText("00 : 00 : 00");
                if (first) {            // when delay is counted down
                    String timeMillis = Long.toString(System.currentTimeMillis() / 1000L);
                    databaseHandler.setValueInExpInfo(timeMillis, "start_time", experimentNumber);

                    if (!gpsLogged) {
                        writeGPSDataToDB(locationHandler.getLatitude(), locationHandler.getLongitude(), locationHandler.getAltitude());
                        Toast.makeText(context, "receivers GPS data logged", Toast.LENGTH_SHORT).show();
                    }

                    findViewById(R.id.experiment_monitor).setVisibility(View.VISIBLE);
                    findViewById(R.id.delaylayout).setVisibility(View.GONE);
                    findViewById(R.id.location_card).setVisibility(View.GONE);

                    connToastsDeactivated = false;

                    hoursD = duration / 3600;
                    int remainder = duration % 3600;
                    minutesD = remainder / 60;
                    secondsD = remainder % 60;

                    startTimers(duration * 1000, findViewById(R.id.exp_time), false);
                } else {        // when experiment duration counted down
                    databaseHandler.setValueInExpInfo(DatabaseHandler.FINISHED, "state", experimentNumber);
                    bleHandler.closeConnection();
                    connected = false;
                    getSupportActionBar().setSubtitle("finished");
                    findViewById(R.id.experiment_monitor).setVisibility(View.GONE);
                    findViewById(R.id.exp_finished_layout).setVisibility(View.VISIBLE);

                    ArrayList<String> experiments = databaseHandler.getPrimaryColumn(false);
                    if(experiments.indexOf(Integer.toString(experimentNumber)) < experiments.size() - 1) {
                        if(databaseHandler.getSingleExperimentInfoValue
                            (Integer.parseInt(experiments.get(experiments.indexOf(Integer.toString(experimentNumber)) + 1)), DatabaseHandler.STATE).equals(DatabaseHandler.READY)) {
                            findViewById(R.id.nextExpButton).setVisibility(View.VISIBLE);
                        }
                    }
                    automaticCsvAndUploadIfAllGPSExistsAndInternetConnection();
                }
            }

        }.start();
    }

    /**
     * extracts the different values of one LoRa parameter, the experiment is supposed to cover, from one byte.
     * example: experiment is supposed to cover spreading factors 7, 8 and 12 (possible spreading factors: 7 to 12).
     * The passed byte will have the following binary representation: 00100011 -> 00 1(sf12) 000 1(sf8) 1(sf7)
     * The parameter combinations are stored encoded in the database. We decode it here for displaying it to the user
     * but send it encoded to the LoRa device.
     *
     * @param param
     * @param bits
     * @return
     */
    private String decodeConfigByte(int param, byte bits) {
        String summary = " ";

        String[] strings1 = getResources().getStringArray(arrayResourceIds[param]);
        for (int i = 0; i < 8; i++) {
            if (((bits >> i) & 1) == 1) {
                summary += strings1[i] + ", ";
                multipliers[param]++;
            }
        }

        summary = summary.substring(0, summary.length() - 2);
        return summary;
    }

    /**
     * prepares the byte array, that is going to be sent to the board, containing all necessary experiment settings information
     *
     * @param total of seconds of delay, with which to start the experiment
     * @return
     */
    private byte[] createSettingsBytes(long total) {
        byte[] delay = Long.toString(total / 1000).getBytes();                         // set delay bytes

        byte[] bytes = new byte[9 + delay.length];

        for (int i = 0; i < 3; i++) {                                           // set msg settings: # of msgs, time between msgs, msg length
            bytes[i] = Byte.parseByte(entry.get(i + DatabaseHandler.NUMBER_OF_MESSAGES));
        }
        bytes[3] = Boolean.parseBoolean(entry.get(DatabaseHandler.AUTO_INCREMENT)) ? (byte) 1 : (byte) 0;     // set auto-increment bit (obsolete)

        for (int i = 4; i < 9; i++) {                                           // set LoRa settings: fr, bw, sf, cr, pw
            bytes[i] = Byte.parseByte(entry.get(i + DatabaseHandler.FREQUENCIES - 4));
        }

        for (int i = 9; i < delay.length + 9; i++) {
            bytes[i] = delay[i - 9];
        }

        return bytes;
    }

    /**
     * fetches the correct experiment from the database and creates the GUI.
     */
    private void initSummary() {
        entry = databaseHandler.getExperimentInfo(experimentNumber);

        // set transmission settings summary
        TextView transmissionSettingsText = findViewById(R.id.transmission_settings);
        String transmissionSettingsOverview = "Number of messages: " + entry.get(DatabaseHandler.NUMBER_OF_MESSAGES) +
                "\nTime between messages: " + entry.get(DatabaseHandler.TIME_BETWEEN_MSGS) +
                "\nMessage length: " + entry.get(DatabaseHandler.MSG_LENGTH) +
                "\nAuto increment: " + entry.get(DatabaseHandler.AUTO_INCREMENT);
        transmissionSettingsText.setText(transmissionSettingsOverview);

        multipliers[5] = Integer.parseInt(entry.get(DatabaseHandler.NUMBER_OF_MESSAGES));
        multipliers[6] = Integer.parseInt(entry.get(DatabaseHandler.TIME_BETWEEN_MSGS));

        // set environment settings summary
        TextView environmentSettingsText = findViewById(R.id.environment_settings);
        String environmentSettingsOverview = "Description: " + entry.get(DatabaseHandler.DESCRIPTION) +
                "\nTarget Distance: " + entry.get(DatabaseHandler.TARGET_DISTANCE) + " m" +
                "\nHeight of Sender: " + entry.get(DatabaseHandler.SENDER_HEIGHT) + " m" +
                "\nHeight of Receiver: " + entry.get(DatabaseHandler.RECEIVER_HEIGHT) + " m" +
                "\nEnvironment: " + entry.get(DatabaseHandler.ENVIRONMENT);
        environmentSettingsText.setText(environmentSettingsOverview);

        // set LoRa settings summary
        TextView loraSettingsText = findViewById(R.id.lora_settings);
        arrayResourceIds = new int[]{R.array.frequency_array, R.array.bandwidth_array, R.array.SF_array, R.array.CR_array, R.array.power_array};
        String loraSettingsOverview = "Frequencies: " + decodeConfigByte(0, Byte.parseByte(entry.get(DatabaseHandler.FREQUENCIES))) +
                "\nBandwidths: " + decodeConfigByte(1, Byte.parseByte(entry.get(DatabaseHandler.BANDWIDTHS))) +
                "\nSpreading factors: " + decodeConfigByte(2, Byte.parseByte(entry.get(DatabaseHandler.SPREADING_FACTORS))) +
                "\nCoding rates: " + decodeConfigByte(3, Byte.parseByte(entry.get(DatabaseHandler.CODING_RATES))) +
                "\nPowers: " + decodeConfigByte(4, Byte.parseByte(entry.get(DatabaseHandler.POWERS)));
        loraSettingsText.setText(loraSettingsOverview);

        // set experiment duration information
        TextView experimentText = findViewById(R.id.exp);
        int totalMsgs = 1;
        duration = 1;
        iterations = 1;

        for (int i = 0; i < multipliers.length; i++) {
            if (i == multipliers.length - 1) {
                duration = totalMsgs * multipliers[i];
                break;
            }
            if (i < 5)
                iterations *= multipliers[i];
            totalMsgs *= multipliers[i];
        }
        duration += 5 * iterations;         // add the 8 seconds between experiment iterations (hardcoded on boards)
        databaseHandler.setValueInExpInfo(Integer.toString(iterations), "number_of_iterations", experimentNumber);
        databaseHandler.setValueInExpInfo(Integer.toString(duration), "duration", experimentNumber);

        String experiment = "Total messages: " + totalMsgs +
                "\nIterations: " + iterations +
                "\nDuration: " + duration + " s";
        experimentText.setText(experiment);

        // initialize other stuff
        hoursEdit = findViewById(R.id.hours_edit);
        minutesEdit = findViewById(R.id.minutes_edit);
        secondsEdit = findViewById(R.id.seconds_edit);
    }

    private void setupCentral() {
        bleHandler = new BLEHandler(this, BLEHandler.RECEIVER, BLEHandler.EXPERIMENT);
        bleHandler.connect();
    }

    /**
     * GUI setup in case the experiment was already conducted.
     */
    private void setupFinishedGUI() {
        getSupportActionBar().setSubtitle("finished");
        findViewById(R.id.countdownpickerlayout).setVisibility(View.GONE);
        findViewById(R.id.start_button).setVisibility(View.GONE);
        findViewById(R.id.ping_button).setVisibility(View.GONE);
        findViewById(R.id.location_card).setVisibility(View.GONE);
        findViewById(R.id.exp_finished_layout).setVisibility(View.VISIBLE);
    }

    /**
     * setup the @LocationHandler for use with this screen (receiver side of the experiment).
     * Pass GPS data about the senders location for calculation of distance.
     */
    private void setupLocationHandler() {
        Location senderLocation = new Location("");
        senderLocation.setLatitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LATITUDE)));
        senderLocation.setLongitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LONGITUDE)));
        senderLocation.setAltitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_ALTITUDE)));
        locationHandler = new LocationHandler(this, findViewById(R.id.latitude),
                findViewById(R.id.longitude), findViewById(R.id.altitude), null,
                findViewById(R.id.distance_to_sender), senderLocation);
    }

    private boolean gpsLogged;

    /**
     * log GPS data and antenna orientation data with button press (if button not pressed before
     * experiment start) method below (writeGPSDataToDB(...)) is called automatically.
     * @param view
     */
    public void logLocation(View view) {
        writeGPSDataToDB(locationHandler.getLatitude(), locationHandler.getLongitude(), locationHandler.getAltitude());
        databaseHandler.setValueInExpInfo(((TextView) findViewById(R.id.accelerate_indicator_rec)).getText().toString(), "receiver_orientation", experimentNumber);
        accelerometerHandler.close();
        Toast.makeText(this, "receivers GPS data logged", Toast.LENGTH_SHORT).show();
        gpsLogged = true;
    }

    /**
     * log GPS data in database.
     * @param lat
     * @param lon
     * @param alt
     */
    private void writeGPSDataToDB(String lat, String lon, String alt) {
        databaseHandler.setValueInExpInfo(lat, "receiver_latitude", experimentNumber);
        databaseHandler.setValueInExpInfo(lon, "receiver_longitude", experimentNumber);
        databaseHandler.setValueInExpInfo(alt, "receiver_altitude", experimentNumber);
    }

    /**
     * send the command to the LoRa receiver to issue a ping request to the LoRa sender,
     * to determine a good location for pre-experiment data exchange
     * @param view
     */
    public void pingDevice(View view) {
        if (!ready) {
            Toast.makeText(this, "devices not ready", Toast.LENGTH_SHORT).show();
            return;
        }
        if (ConfigSetter.config[4] < 4)       // see strings resource file for assignment of power values to ConfigSetter.config
            connToastsDeactivated = true;
        bleHandler.writeExperimentSettingsCharacteristic("!".getBytes());
    }

    /**
     * the method name is too long but speaks for itself.
     */
    private void automaticCsvAndUploadIfAllGPSExistsAndInternetConnection() {
        if(!isConnectedToInternet()) {
            Toast.makeText(context, "Not connected to the internet, upload data later", Toast.LENGTH_SHORT).show();
            return;
        }
        if(!allGPSDataExists()) {
            Toast.makeText(context, "Some GPS data is missing, upload data later", Toast.LENGTH_SHORT).show();
            return;
        }
        try {
            databaseHandler.makeCSV(experimentNumber);
            enterDiffValuesIntoDB();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * checks if all 6 GPS datapoints (2x latitude, 2x longitude, 2x altitude) are logged.
     * (if not GPS malfunctioned or an experiment was started remotely, that did neither prepare
     * the sender itself nor was started from the end of another experiment.
     * @return
     */
    private boolean allGPSDataExists() {
        for(int i = 0; i < 6; i++) {
            if(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LATITUDE).equals("-1"))
                return false;
        }
        return true;
    }

    /**
     * called if GPS data is missing
     * @param view
     */
    public void createCsv(View view) {
        try {
            databaseHandler.makeCSV(experimentNumber);
        } catch (IOException e) {
            e.printStackTrace();
        }
        completeLocationValues(DatabaseHandler.SENDER_LATITUDE);
    }

    /**
     * checks column after column with an offset to the GPS columns in the experiment information table.
     * Shows an alert dialog to manually enter missing data for each missing data point.
     * @param column
     */
    private void completeLocationValues(int column) {
        if (column > DatabaseHandler.RECEIVER_ALTITUDE) {
            enterDiffValuesIntoDB();
            return;
        }

        if (!databaseHandler.getSingleExperimentInfoValue(experimentNumber, column).equals("-1")) {
            completeLocationValues(column + 1);
        } else {
            final EditText editText = new EditText(this);
            String locationIdentifier = getResources().getStringArray(R.array.db_location_identifiers)[column - DatabaseHandler.SENDER_LATITUDE];
            new AlertDialog.Builder(this)
                    .setTitle("Missing Location Value")
                    .setMessage("Please enter a value for " + locationIdentifier)
                    .setCancelable(true)
                    .setView(editText)
                    .setPositiveButton("confirm", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            String loc = editText.getText().toString();
                            databaseHandler.setValueInExpInfo(loc, locationIdentifier, experimentNumber);
                            completeLocationValues(column + 1);
                        }
                    }).show();
        }
    }

    /**
     * once all GPS data is available this method calculates and logs the
     * differences/distances between receiver and sender
     */
    private void enterDiffValuesIntoDB() {
        Location senderLocation = new Location("");
        senderLocation.setLatitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LATITUDE)));
        senderLocation.setLongitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LONGITUDE)));

        Location receiverLocation = new Location("");
        receiverLocation.setLatitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.RECEIVER_LATITUDE)));
        receiverLocation.setLongitude(Double.parseDouble(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.RECEIVER_LONGITUDE)));
        String distance = Float.toString(senderLocation.distanceTo(receiverLocation));
        databaseHandler.setValueInExpInfo(distance, "actual_distance", experimentNumber);

        float altDiff = Float.parseFloat(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_ALTITUDE))
                - Float.parseFloat(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.RECEIVER_ALTITUDE));
        String altitudeDifference = Float.toString(altDiff);
        databaseHandler.setValueInExpInfo(altitudeDifference, "altitude_difference", experimentNumber);

        getWeatherData();
    }

    /**
     * prepares a url to get the weather for the time and location of the experiment.
     * it starts the thread, on which the weather is downloaded, the experiment information is completed and
     * uploaded alongside the experiment results.
     */
    private void getWeatherData() {
        System.out.println("at getWeather");
        String url = "https://api.openweathermap.org/data/2.5/onecall/timemachine?lat=";
        url += databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LATITUDE);
        url += "&lon=" + databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LONGITUDE);
        url += "&dt=" + databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.START_TIME).substring(0, 10);
        url += "&appid=your openweathermap id";
        new ClientThread(ClientThread.GET_WEATHER, new String[]{url, Integer.toString(experimentNumber)}, this).start();
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
                        if (!connToastsDeactivated)
                            Toast.makeText(context, "connected to device", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                    } else {
                        ready = false;
                        if (!connToastsDeactivated)
                            Toast.makeText(context, "disconnected", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.disconnected));
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.CONNECTION_STATE_CHANGED");
        registerReceiver(connStateBroadcastReceiver, filter);
    }

    private String[] dbEntries;

    /**
     * sets up a broadcast receiver for notifications about the rssi and snr values determined at the receiver.
     * Deals with different order of arrival of rssi/snr and message header.
     */
    private void setupRSSIsNRBroadcastReceiver() {
        broadcastReceiverRSSI = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.RSSI_RECEIVED".equals(intent.getAction())) {
                    byte[] bytes = intent.getByteArrayExtra("com.example.loravisual.RSSISNR");
                    int rssi = LVUtil.makeByteUnsigned(bytes[1]);
                    int bitErrors = LVUtil.makeByteUnsigned(bytes[3]);
                    String rssiSnr = "RSSI: -" + rssi + "     SNR: " + (bytes[2]);
                    rssiSnrTextView.setText(rssiSnr);
                    if (dbEntries == null) {
                        dbEntries = new String[5];
                        dbEntries[2] = "-" + rssi;
                        dbEntries[3] = Byte.toString(bytes[2]);
                        dbEntries[4] = Integer.toString(bitErrors);
                    } else {
                        dbEntries[2] = "-" + rssi;
                        dbEntries[3] = Byte.toString(bytes[2]);
                        dbEntries[4] = Integer.toString(bitErrors);
                        databaseHandler.logReceivedTransmission(Integer.toString(experimentNumber), dbEntries);
                        dbEntries = null;
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.RSSI_RECEIVED");
        registerReceiver(broadcastReceiverRSSI, filter);

    }

    /**
     * sets up a broadcast receiver for notifications on received LoRa messages.
     * manages the information display on the most recently received LoRa transmission.
     */
    private void setupMsgBroadcastReceiver() {
        broadcastReceiverLoraMsg = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.MSG_RECEIVED".equals(intent.getAction())) {
                    byte[] bytes = intent.getByteArrayExtra("com.example.loravisual.MSG");

                    String totalNumber = Integer.toString(bytes[11] + (bytes[10] * multipliers[5])); // + correct total message number calculated from two ble transmitted bytes (#msg number in iteration = [11], (#iteration = [10])
                    String msg = new String(bytes).substring(0, 10);                                // msg = header (ble transmitted from board as string

                    if (colorToggle) {
                        msgTextView.setTextColor(getResources().getColor(R.color.colorPrimaryDark, null));
                    } else {
                        msgTextView.setTextColor(getResources().getColor(R.color.white, null));
                    }
                    colorToggle = !colorToggle;
                    msgTextView.setText(msg + " (" + totalNumber + ")");

                    if (dbEntries == null) {
                        dbEntries = new String[5];
                        dbEntries[0] = totalNumber;
                        dbEntries[1] = msg;
                    } else {
                        dbEntries[0] = totalNumber;
                        dbEntries[1] = msg;
                        databaseHandler.logReceivedTransmission(Integer.toString(experimentNumber), dbEntries);
                        dbEntries = null;
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.MSG_RECEIVED");
        registerReceiver(broadcastReceiverLoraMsg, filter);
    }

    /**
     * receives various notifications, in the form of negative integers,
     * displays appropriate toast.
     */
    private void setupGeneralBroadcastReceiver() {
        generalBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if ("com.example.loravisual.GENERAL_BROADCAST".equals(intent.getAction())) {
                    int status = intent.getIntExtra("com.example.loravisual.GENERAL", 0);
                    if (status == -2) {
                        if (!connToastsDeactivated)
                            Toast.makeText(context, "LoRa configuration synchronized, experiment ready", Toast.LENGTH_SHORT).show();
                        getSupportActionBar().setSubtitle(getString(R.string.connected));
                        ready = true;
                    } else if (status == -3) {
                        Toast.makeText(context, "correct response received", Toast.LENGTH_SHORT).show();
                    } else if (status == -4) {
                        Toast.makeText(context, "response received but did not match", Toast.LENGTH_SHORT).show();
                    } else if (status == -7) {
                        Toast.makeText(context, "no response received", Toast.LENGTH_SHORT).show();
                    } else if (status == -5) {
                        Toast.makeText(context, "no ACK received, trying again", Toast.LENGTH_SHORT).show();
                    } else if (status == -6) {
                        Toast.makeText(context, "ACK received", Toast.LENGTH_SHORT).show();
                        startTimers(total, findViewById(R.id.htext), true);
                    }
                    if(status == -3 || status == -4 || status == -7)
                        connToastsDeactivated = false;
                }
            }
        };
        IntentFilter filter = new IntentFilter("com.example.loravisual.GENERAL_BROADCAST");
        registerReceiver(generalBroadcastReceiver, filter);
    }

    /**
     * brings up the same screen, only for the next experiment in line. Delay and other user
     * input on this screen must be configured again, sender GPS data from the last experiment
     * is preserved and logged.
     * @param view
     */
    public void startNextExperiment(View view) {
        finish();
        Intent intent = new Intent(this, StartExperimentScreen.class);

        ArrayList<String> experiments = databaseHandler.getPrimaryColumn(false);
        int indexOfThisExperiment = experiments.indexOf(Integer.toString(experimentNumber));
        int nextExperiment = Integer.parseInt(experiments.get(indexOfThisExperiment + 1));

        databaseHandler.setValueInExpInfo(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LATITUDE), "sender_latitude", nextExperiment);
        databaseHandler.setValueInExpInfo(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_LONGITUDE), "sender_longitude", nextExperiment);
        databaseHandler.setValueInExpInfo(databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.SENDER_ALTITUDE), "sender_altitude", nextExperiment);

        intent.putExtra("absolutePos", nextExperiment);
        startActivity(intent);
    }

    /**
     * check if smartphone has internet connection.
     * @return
     */
    private boolean isConnectedToInternet() {
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        return cm.getActiveNetworkInfo() != null && cm.getActiveNetworkInfo().isConnected();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(timer != null) {
            timer.cancel();
            timer = null;
        }

        if (justShowingInfo)
            return;

        bleHandler.closeConnection();
        unregisterReceiver(connStateBroadcastReceiver);
        unregisterReceiver(broadcastReceiverLoraMsg);
        unregisterReceiver(broadcastReceiverRSSI);
        unregisterReceiver(generalBroadcastReceiver);

        if (connected)
            Toast.makeText(this, "disconnected", Toast.LENGTH_SHORT).show();
    }

    // make sure back button wasn't pressed accidentally during experiment
    @Override
    public void onBackPressed() {
        if(!databaseHandler.getSingleExperimentInfoValue(experimentNumber, DatabaseHandler.STATE).equals(DatabaseHandler.FINISHED)) {
            new AlertDialog.Builder(this)
                    .setTitle("Cancel experiment?")
                    .setCancelable(false)
                    .setPositiveButton("yes", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    })
                    .setNegativeButton("no", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            return;
                        }
                    }).show();
        } else {
            finish();
        }
    }
}