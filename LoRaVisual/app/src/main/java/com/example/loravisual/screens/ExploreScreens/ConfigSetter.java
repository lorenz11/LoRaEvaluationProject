package com.example.loravisual.screens.ExploreScreens;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.example.loravisual.R;

import java.util.ArrayList;

/**
 * this class manages the spinners, used to control the LoRa settings.
 */
public class ConfigSetter {
    private static final String TAG = "at ConfigSetter";
    // we use this array to read the currently desired LoRa parameter combinations from anywhere in the code.
    public static byte[] config = {0, 0, 3, 0, 0};
    static ArrayList<Spinner> spinners;

    /**
     * sets the spinners in the appropriate activity.
     * @param activity
     */
    public static void setExploreSpinners(Activity activity) {
        spinners = new ArrayList<>();
        ArrayList<ArrayAdapter<CharSequence>> adapters = new ArrayList<>();

        int[] spinnerIds = {R.id.frequ_spinner, R.id.bandw_spinner, R.id.sf_spinner, R.id.cr_spinner, R.id.power_spinner};
        int[] stringArrayIds = {R.array.frequency_array, R.array.bandwidth_array, R.array.SF_array, R.array.CR_array, R.array.power_array};
        int[] selections = {0, 0, 3, 0, 7};

        for (int i = 0; i < 5; i++) {
            spinners.add((Spinner) activity.findViewById(spinnerIds[i]));
            adapters.add(ArrayAdapter.createFromResource(activity,
                    stringArrayIds[i], android.R.layout.simple_spinner_item));
            adapters.get(i).setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinners.get(i).setAdapter(adapters.get(i));
            spinners.get(i).setSelection(selections[i]);

            int finalI = i;
            spinners.get(i).setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view,
                                           int position, long id) {
                    Log.i(TAG, (String) parent.getItemAtPosition(position));
                    config[finalI] = (byte) position;
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {}
            });
        }
    }

    /**
     * set spinners enabled/disabled.
     * @param enable
     */
    public static void enableSpinners(boolean enable) {
        for(int i = 0; i < 5; i++) {
            spinners.get(i).setEnabled(enable);
        }
    }


}
