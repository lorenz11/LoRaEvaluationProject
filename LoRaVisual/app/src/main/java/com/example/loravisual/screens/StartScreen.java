package com.example.loravisual.screens;

import android.Manifest;
import android.content.Intent;
import android.location.Location;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.loravisual.R;
import com.example.loravisual.async.ClientThread;
import com.example.loravisual.database.DatabaseHandler;
import com.example.loravisual.lvutilities.FirebaseHandler;
import com.example.loravisual.lvutilities.LVUtil;
import com.example.loravisual.screens.ExperimentScreens.ExperimentOverviewScreen;
import com.example.loravisual.screens.ExploreScreens.DeviceScreen;

import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * this class is the activity representing the start screen of the app.
 */
public class StartScreen extends AppCompatActivity {
    private static final String TAG = "at StartScreen";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_start_screen);

        // permission required by Androids specifications
        ActivityCompat.requestPermissions(StartScreen.this,
                new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_BACKGROUND_LOCATION, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                1);
    }

    /**
     * starts an options screen for explore mode.
     * @param view button to press.
     */
    public void toExplore(View view) {
        Intent intent = new Intent(this, DeviceScreen.class);
        startActivity(intent);
    }

    /**
     * brings up the experiment overview
     * @param view
     */
    public void toExperiment(View view) {
        Intent intent = new Intent(this, ExperimentOverviewScreen.class);
        startActivity(intent);
    }
}
