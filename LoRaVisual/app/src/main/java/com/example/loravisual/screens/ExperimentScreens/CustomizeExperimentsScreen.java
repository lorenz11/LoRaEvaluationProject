package com.example.loravisual.screens.ExperimentScreens;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.Toast;

import com.example.loravisual.R;
import com.example.loravisual.database.DatabaseHandler;
import com.example.loravisual.lvutilities.Encoder;

import java.util.ArrayList;

/**
 * This screen class lets a user define the parameters of an experiment.
 */
public class CustomizeExperimentsScreen extends AppCompatActivity {
    private static final String TAG = "at CustomizeExperimentsScreen";

    DatabaseHandler databaseHandler = new DatabaseHandler(this);

    // resource ID arrays for check boxes to set various LoRa parameter combinations.
    ArrayList<CheckBox> frequencies = new ArrayList<>();
    int[] frIds = {R.id.f8695, R.id.f8681, R.id.f8683, R.id.f8685, R.id.f8671, R.id.f8673, R.id.f8675, R.id.f8677};

    ArrayList<CheckBox> bandwidths = new ArrayList<>();
    int[] bwIds = {R.id.bw125, R.id.bw250};

    ArrayList<CheckBox> spreadingFactors = new ArrayList<>();
    int[] sfIds = {R.id.sf_7, R.id.sf_8, R.id.sf_9, R.id.sf_10, R.id.sf_11, R.id.sf_12};

    ArrayList<CheckBox> codingRates = new ArrayList<>();
    int[] crIds = {R.id.cr_4_5, R.id.cr_4_6, R.id.cr_4_7, R.id.cr_4_8};

    ArrayList<CheckBox> powers = new ArrayList<>();
    int[] pwIds = {R.id.p5, R.id.p6, R.id.p7, R.id.p8, R.id.p9, R.id.p10, R.id.p11, R.id.p12};

    // views for editing other information
    EditText noMsgsEdit;
    EditText timeBetweenEdit;
    EditText msgLengthEdit;
    EditText seedEdit;
    Switch autoIncrementSwitch;

    EditText descriptionEdit;
    EditText distanceEdit;
    EditText heightSenderEdit;
    EditText heightRecEdit;
    EditText environmentEdit;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_customize_experiments_screen);
        getSupportActionBar().setTitle("Create Experiment");

        noMsgsEdit = findViewById(R.id.no_msgs_edit);
        timeBetweenEdit = findViewById(R.id.time_edit);
        msgLengthEdit = findViewById(R.id.msg_length_edit);
        seedEdit = findViewById(R.id.seed_edit);
        autoIncrementSwitch = findViewById(R.id.auto_incr_switch);

        descriptionEdit = findViewById(R.id.descr_edit);
        distanceEdit = findViewById(R.id.dist_edit);
        heightSenderEdit = findViewById(R.id.h_sender_edit);
        heightRecEdit = findViewById(R.id.h_rec_edit);
        environmentEdit = findViewById(R.id.env_edit);



        for(int i = 0; i < frIds.length; i++) {
            frequencies.add(findViewById(frIds[i]));
        }
        for(int i = 0; i < bwIds.length; i++) {
            bandwidths.add(findViewById(bwIds[i]));
        }
        for(int i = 0; i < sfIds.length; i++) {
            spreadingFactors.add(findViewById(sfIds[i]));
        }
        for(int i = 0; i < crIds.length; i++) {
            codingRates.add(findViewById(crIds[i]));
        }
        for(int i = 0; i < pwIds.length; i++) {
            powers.add(findViewById(pwIds[i]));
        }
    }

    /**
     * triggered when "create" button pressed, checks all entered information for user mistakes,
     * then logs it.
     * @param view
     */
    public void experimentReady(View view) {
        String noMsgs = noMsgsEdit.getText().toString();
        if(noMsgs.equals("") || Integer.parseInt(noMsgs) < 1 || Integer.parseInt(noMsgs) > 100) {
            Toast.makeText(this, "Please specify the number of messages (between 1 and 100)", Toast.LENGTH_SHORT).show();
            return;
        }

        String timeBetween = timeBetweenEdit.getText().toString();
        if(timeBetween.equals("")) {
            Toast.makeText(this, "Please specify the time between messages", Toast.LENGTH_SHORT).show();
            return;
        }

        String msgLength = msgLengthEdit.getText().toString();
        if(msgLength.equals("") || Integer.parseInt(msgLength) < 11 || Integer.parseInt(noMsgs) > 100) {
            Toast.makeText(this, "Please specify a message length between 11 and 100", Toast.LENGTH_SHORT).show();
            return;
        }

        String seed = "0";

        String autoIncrement = Boolean.toString(autoIncrementSwitch.isChecked());

        String description = descriptionEdit.getText().toString();

        String distance = distanceEdit.getText().toString();
        if(distance.equals("")) {
            Toast.makeText(this, "Please specify the distance", Toast.LENGTH_SHORT).show();
            return;
        }

        String heightSender = heightSenderEdit.getText().toString();
        if(distance.equals("")) {
            Toast.makeText(this, "Please specify the sender height", Toast.LENGTH_SHORT).show();
            return;
        }

        String heightReceiver = heightRecEdit.getText().toString();
        if(distance.equals("")) {
            Toast.makeText(this, "Please specify the receiver height", Toast.LENGTH_SHORT).show();
            return;
        }

        String environment = environmentEdit.getText().toString();
        if(environment.equals("")) {
            Toast.makeText(this, "Please specify the environment", Toast.LENGTH_SHORT).show();
            return;
        }

        if(!allParametersHaveOneChecked()) {
            Toast.makeText(this, "Please check at least one box in every category", Toast.LENGTH_SHORT).show();
            return;
        }

        boolean[] frBits = new boolean[frIds.length];
        for(int i = 0; i < frIds.length; i++) {
            frBits[i] = frequencies.get(i).isChecked();
            System.out.println(frequencies.get(i).isChecked());
        }
        String frByte = Byte.toString(Encoder.encodeParamByte(frBits));

        boolean[] bwBits = new boolean[bwIds.length];
        for(int i = 0; i < bwIds.length; i++) {
            bwBits[i] = bandwidths.get(i).isChecked();
        }
        String bwByte = Byte.toString(Encoder.encodeParamByte(bwBits));

        boolean[] sfBits = new boolean[sfIds.length];
        for(int i = 0; i < sfIds.length; i++) {
            sfBits[i] = spreadingFactors.get(i).isChecked();
        }
        String sfByte = Byte.toString(Encoder.encodeParamByte(sfBits));

        boolean[] crBits = new boolean[crIds.length];
        for(int i = 0; i < crIds.length; i++) {
            crBits[i] = codingRates.get(i).isChecked();
        }
        String crByte = Byte.toString(Encoder.encodeParamByte(crBits));

        boolean[] pwBits = new boolean[pwIds.length];
        for(int i = 0; i < pwIds.length; i++) {
            pwBits[i] = powers.get(i).isChecked();
        }
        String pwByte = Byte.toString(Encoder.encodeParamByte(pwBits));


        databaseHandler.logExperiment(noMsgs, timeBetween, msgLength, seed, autoIncrement,
                description, distance, heightSender, heightReceiver, environment, DatabaseHandler.READY,
                frByte, bwByte, sfByte, crByte, pwByte);

        databaseHandler.createTableForIterations(Integer.toString(databaseHandler.getLastPos()));

        if(((CheckBox) findViewById(R.id.start_now_cb)).isChecked()) {
            Intent intent = new Intent(this, PrepareSenderScreen.class);
            intent.putExtra("absolutePos", databaseHandler.getLastPos());
            startActivity(intent);
        } else {
            finish();
        }
    }

    /**
     * method to check if at least one checkbox per category is checked.
     * @return
     */
    private boolean allParametersHaveOneChecked() {
        ArrayList<CheckBox> [] checkBoxLists = new ArrayList[]{frequencies, bandwidths, spreadingFactors, codingRates, powers};

        boolean ok = false;
        for(ArrayList<CheckBox> list : checkBoxLists) {
            for(CheckBox checkBox : list) {
                if(checkBox.isChecked()) {
                    ok = true;
                    break;
                }
            }
            if(!ok)
                return ok;
            ok = false;
        }
        return true;
    }
}