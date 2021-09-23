package com.example.loravisual.lvutilities;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.Button;
import android.widget.TextView;

import java.text.DecimalFormat;

/**
 * This class facilitates the use of the smartphone's accelerometer (using the Android API)
 */
public class AccelerometerHandler implements SensorEventListener {
    TextView view;
    DecimalFormat df;
    Context context;

    public AccelerometerHandler(Context context, TextView view) {
        df = new DecimalFormat("0.00");     // cut some decimal places
        this.context = context;
        this.view = view;
        SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        if(manager.getSensorList(Sensor.TYPE_ACCELEROMETER).size() != 0) {
            Sensor accelerometer = manager.getSensorList(Sensor.TYPE_ACCELEROMETER).get(0);
            manager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_GAME);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {}

    int i = 0;

    /**
     * we change the view, that displays the smartphone's orientation in 3D space.
     * (but only every 14th time it changes, as otherwise the frequency of changes is too high)
     * @param event
     */
    public void onSensorChanged(SensorEvent event) {
        if(i == 14) {
            view.setText("x: " + df.format(event.values[0]) + ", y: " + df.format(event.values[1]) + ", z: " + df.format(event.values[2]));
            i = 0;
        }
        i++;
    }

    /**
     * we need to close the listener.
     */
    public void close() {
        SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        manager.unregisterListener(this);
        manager.flush(this);
    }
}