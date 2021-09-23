package com.example.loravisual.database;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import com.example.loravisual.lvutilities.FirebaseHandler;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;


public class DatabaseHandler {
    private static final String TAG = "at DatabaseHandler";

    // identifiers for database columns
    public static final int NUMBER_OF_MESSAGES = 1;
    public static final int TIME_BETWEEN_MSGS = 2;
    public static final int MSG_LENGTH = 3;
    public static final int PAYLOAD_SEED = 4;
    public static final int AUTO_INCREMENT = 5;

    public static final int DESCRIPTION = 6;
    public static final int TARGET_DISTANCE = 7;
    public static final int SENDER_HEIGHT = 8;
    public static final int RECEIVER_HEIGHT = 9;
    public static final int ENVIRONMENT = 10;

    public static final int STATE = 11;

    public static final int FREQUENCIES = 12;
    public static final int BANDWIDTHS = 13;
    public static final int SPREADING_FACTORS = 14;
    public static final int CODING_RATES = 15;
    public static final int POWERS = 16;


    public static final int NUMBER_OF_ITERATIONS = 17;
    public static final int DURATION = 18;


    public static final int START_TIME = 19;
    public static final int SENDER_LATITUDE = 20;
    public static final int SENDER_LONGITUDE = 21;
    public static final int SENDER_ALTITUDE = 22;
    public static final int RECEIVER_LATITUDE = 23;
    public static final int RECEIVER_LONGITUDE = 24;
    public static final int RECEIVER_ALTITUDE = 25;
    public static final int ACTUAL_DISTANCE = 26;
    public static final int ALTITUDE_DIFFERENCE = 27;
    public static final int TEMPERATURE_VALUES = 28;
    public static final int HUMIDITY_VALUES = 29;
    public static final int PRESSURE_VALUES = 30;
    public static final int WEATHER_DESCRIPTIONS = 31;

    public static final int SENDER_ORIENTATION = 32;
    public static final int RECEIVER_ORIENTATION = 32;

    // other constants
    public static final int EXPERIMENT_VALUE_COUNT = 34;        // with _id
    public static final int EXPERIMENT_DATA_VALUE_COUNT = 5;    // without _id

    public static final String READY = "ready";
    public static final String FINISHED = "finished";

    DBHelper helper;
    Cursor cursor;

    Context context;

    /**
     * this constructor creates a table with 34 columns, to store information about the experiment
     * (if it was not already created)
     * @param context
     */
    public DatabaseHandler(Context context) {
        String createString = "CREATE TABLE IF NOT EXISTS Experiments "
                + "(_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                + "number_of_messages TEXT NOT NULL, "
                + "time_between_msgs TEXT NOT NULL, "
                + "msg_length TEXT NOT NULL, "
                + "payload_seed TEXT NOT NULL, "
                + "auto_increment TEXT NOT NULL, "

                + "description TEXT NOT NULL, "
                + "target_distance TEXT NOT NULL, "
                + "sender_height TEXT NOT NULL, "
                + "receiver_height TEXT NOT NULL, "
                + "environment TEXT NOT NULL, "
                + "state TEXT NOT NULL, "             // ready, started, finished

                + "frequencies TEXT NOT NULL, "
                + "bandwidths TEXT NOT NULL, "
                + "spreading_factors TEXT NOT NULL, "
                + "coding_rates TEXT NOT NULL, "
                + "powers TEXT NOT NULL,"

                + "number_of_iterations TEXT, "
                + "duration TEXT, "

                + "start_time TEXT, "
                + "sender_latitude TEXT, "
                + "sender_longitude TEXT, "
                + "sender_altitude TEXT, "
                + "receiver_latitude TEXT, "
                + "receiver_longitude TEXT, "
                + "receiver_altitude TEXT, "
                + "actual_distance TEXT, "
                + "altitude_difference TEXT, "
                + "temperature_values TEXT, "
                + "humidity_values TEXT, "
                + "pressure_values TEXT, "
                + "weather_descriptions TEXT, "
                + "sender_orientation TEXT,"
                + "receiver_orientation TEXT);";
        helper = new DBHelper(context, "Log", null, 1, createString);
        this.context = context;
    }

    /**
     * logs the parts of experiment info, we specify in the experiment customization screen
     */
    public void logExperiment(String numberOfMessages, String timeBetweenMsgs, String msgLength, String payloadSeed, String autoIncrement,
                              String description, String targetDistance, String senderHeight, String receiverHeight, String environment, String state,
                              String frequencies, String bandwidths, String spreadingFactors, String codingRates, String powers) {
        Log.i(TAG, "logged experiment into database.");
        SQLiteDatabase database = helper.getWritableDatabase();
        ContentValues values = new ContentValues();

        values.put("number_of_messages", numberOfMessages);
        values.put("time_between_msgs", timeBetweenMsgs);
        values.put("msg_length", msgLength);
        values.put("payload_seed", payloadSeed);
        values.put("auto_increment", autoIncrement);

        values.put("description", description);
        values.put("target_distance", targetDistance);
        values.put("sender_height", senderHeight);
        values.put("receiver_height", receiverHeight);
        values.put("environment", environment);
        values.put("state", state);     // don't forgeet to adjust EXPERIMENT_VALUE_COUNT when adding more columns

        values.put("frequencies", frequencies);
        values.put("bandwidths", bandwidths);
        values.put("spreading_factors", spreadingFactors);
        values.put("coding_rates", codingRates);
        values.put("powers", powers);

        database.insert("Experiments", "", values);
        values.clear();
        database.close();
    }

    /**
     * returns entry from the experiments information table
     * @param absPos
     * @return
     */
    public ArrayList<String> getExperimentInfo(int absPos) {
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM Experiments WHERE _id = " + absPos + ";"), null);
        cursor.moveToFirst();

        ArrayList<String> entry = new ArrayList<>();
        for (int i = 0; i < EXPERIMENT_VALUE_COUNT; i++) {
            entry.add(cursor.getString(i));
        }
        cursor.close();
        database.close();

        return entry;
    }

    /**
     * returns the value of a single column and row
     * @param absPos
     * @param column
     * @return
     */
    public String getSingleExperimentInfoValue(int absPos, int column) {
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM Experiments WHERE _id = " + absPos + ";"), null);
        cursor.moveToFirst();
        String value = cursor.getString(column);
        cursor.close();
        database.close();

        return value;
    }

    /**
     * updates a given value. we mostly use this to log experiment information, that was not known at the time of creation.
     * @param update
     * @param column
     * @param experimentID
     */
    public void setValueInExpInfo(String update, String column, int experimentID) {
        SQLiteDatabase database = helper.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(column, update);
        String[] args = new String[]{Integer.toString(experimentID)};
        database.update("Experiments", values, "_id=?", args);
        values.clear();
        database.close();
    }

    /**
     * the experiment overview screen shows experiment numbers. This returns the sequence of
     * ordered experiment numbers, without deleted experiments.
     * @param withTitle
     * @return
     */
    public ArrayList<String> getPrimaryColumn(boolean withTitle) {
        ArrayList<String> list = new ArrayList<>();
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM Experiments;"), null);
        cursor.moveToFirst();
        int cursorCount = cursor.getCount();
        for (int i = 0; i < cursorCount; i++) {
            if (withTitle)
                list.add("Experiment " + Integer.toString(cursor.getInt(0)));
            else
                list.add(Integer.toString(cursor.getInt(0)));
            cursor.moveToNext();
        }

        cursor.close();
        database.close();

        return list;
    }

    /**
     * this returns the last experiments position, which is needed for creating the table for result data.
     * @return
     */
    public int getLastPos() {
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM Experiments;"), null);
        cursor.moveToLast();
        int lastPos = cursor.getInt(0);

        cursor.close();
        database.close();

        return lastPos;
    }

    /**
     * just for debugging
     */
    public void getTableNames() {
        SQLiteDatabase database = helper.getReadableDatabase();
        Cursor cursor = database.rawQuery("SELECT name FROM sqlite_master WHERE type='table' AND name!='android_metadata' order by name", null);
        cursor.moveToFirst();
        for(int i = 0; i < cursor.getCount(); i++) {
            System.out.println("table "+ i + ": " + cursor.getString(0));
            cursor.moveToNext();
        }
    }

    /**
     * delete entry in Experiments overview table and delete corresponding data table
     * @param absPos
     */
    public void deleteExperiment(int absPos) {
        SQLiteDatabase database = helper.getReadableDatabase();
        database.delete("Experiments", "_id = " + absPos, null);
        database.delete("Experiment" + absPos, null, null);
    }

    /**
     * creates a new table to store the results of an experiment.
     * @param expNumber
     */
    public void createTableForIterations(String expNumber) {
        Log.i(TAG, "created database for experiment " + expNumber);
        String createString = "CREATE TABLE IF NOT EXISTS Experiment" + expNumber + " "
                + "(_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                + "totalnumber TEXT, "
                + "identifier TEXT, "
                + "rssi TEXT, "
                + "snr TEXT, "
                + "biterrors TEXT);";
        helper = new DBHelper(context, "Log", null, 1, createString);
        SQLiteDatabase database = helper.getWritableDatabase();
        helper.onCreate(database);
    }

    /**
     * for debugging only
     * @param expNumber
     * @param column
     * @param row
     * @return
     */
    public String getSingleExperimentDataValue(int expNumber, int column, int row) {
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM experiment" + expNumber + " WHERE _id = " + row + ";"), null);
        cursor.moveToFirst();
        String value = cursor.getString(column);
        cursor.close();
        database.close();

        return value;
    }

    /**
     * logs the various results of a single transmission in the appropriate table
     * @param expNumber
     * @param entries
     */
    public void logReceivedTransmission(String expNumber, String [] entries) {
        Log.i(TAG, "logged transmission into database.");
        SQLiteDatabase database = helper.getWritableDatabase();
        ContentValues values = new ContentValues();

        values.put("totalnumber", entries[0]);
        values.put("identifier", entries[1]);
        values.put("rssi", entries[2]);
        values.put("snr", entries[3]);
        values.put("biterrors", entries[4]);

        database.insert("Experiment" + expNumber, "", values);
        values.clear();
        database.close();
    }

    /**
     * creates a text file from one row of the experiment info table for upload to firebase.
     * @param experimentNumber
     * @throws IOException
     */
    public void makeExpInfoText(int experimentNumber) throws IOException {
        File root = context.getExternalFilesDir(null);
        File file = new File(root, "experiment" + experimentNumber + "/experiment_info" + experimentNumber + ".txt");

        ArrayList<String> experiment = getExperimentInfo(experimentNumber);
        FileOutputStream stream = new FileOutputStream(file);

        for(int i = 0; i < experiment.size(); i++) {
            stream.write((experiment.get(i) + "\n").getBytes());
        }
        stream.close();
    }

    /**
     * creates a CSV file, containing experiment result for upload to firebase.
     * @param experimentNumber
     * @throws IOException
     */
    public void makeCSV(int experimentNumber) throws IOException {
        String dir = "/storage/emulated/0/Android/data/com.example.loravisual/files/experiment" + experimentNumber;
        File expDirectory = new File(dir);
        expDirectory.mkdir();

        File root = context.getExternalFilesDir(null);
        File file = new File(root, "experiment" + experimentNumber + "/experiment_data" + experimentNumber + ".csv");

        FileOutputStream stream = new FileOutputStream(file);
        SQLiteDatabase database = helper.getReadableDatabase();
        cursor = database.rawQuery(("SELECT * FROM Experiment" + experimentNumber + ";"), null);


        // create column name line of CSV file
        String line = "";
        for(int i = 1; i <= EXPERIMENT_DATA_VALUE_COUNT; i++) {
            line += cursor.getColumnName(i);
            if(i == EXPERIMENT_DATA_VALUE_COUNT) {
                line += "\n";
                break;
            }
            line += ",";
        }
        stream.write(line.getBytes());
        line = "";

        // create lines with results
        cursor.moveToFirst();
        int cursorCount = cursor.getCount();
        for (int i = 0; i < cursorCount; i++) {
            for(int j = 1; j <= EXPERIMENT_DATA_VALUE_COUNT; j++) {
                line += cursor.getString(j);
                if(j == EXPERIMENT_DATA_VALUE_COUNT){
                    line += "\n";
                    break;
                }
                line += ",";
            }
            stream.write(line.getBytes());
            line = "";
            cursor.moveToNext();
        }

        stream.close();

        cursor.close();
        database.close();
    }
}
