package com.example.loravisual.screens.ExploreScreens;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.example.loravisual.R;

/**
 * choose which board to control on this screen.
 */
public class DeviceScreen extends AppCompatActivity {
    private static final String TAG = "at DeviceScreen";

    int device;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_screen);

        getSupportActionBar().setTitle("Devices");
        getSupportActionBar().setBackgroundDrawable(new ColorDrawable(ContextCompat.getColor(this, R.color.colorPrimaryDark)));

        Window window = this.getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.setStatusBarColor(ContextCompat.getColor(this, R.color.colorPrimary));
    }

    /**
     * brings up the controller screen for the sending board.
     * @param view
     */
    public void setForSenderDevice(View view) {
        Intent intent = new Intent(this, SenderExploreScreen.class);
        startActivity(intent);
    }

    /**
     * brings up the controller and monitoring screen for the receiving board.
     * @param view
     */
    public void setForReceiverDevice(View view) {
        Intent intent = new Intent(this, ReceiverExploreScreen.class);
        startActivity(intent);
    }
}