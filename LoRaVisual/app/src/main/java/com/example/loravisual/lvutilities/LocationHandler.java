package com.example.loravisual.lvutilities;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

/**
 * This class handles the retrieval of GPS data for slightly varying cases.
 */
public class LocationHandler implements LocationListener {
    LocationManager locationManager;
    Location currentLocation;

    Context context;
    TextView latTextView;
    TextView longTextView;
    TextView altTextView;

    // used in experiments to calculate quality indicator in explore mode for tagged distance
    Location taggedLocation;

    // used in explore mode for tagged distance
    TextView distTextView;

    // only if for receiver in experiment mode
    Location senderLocation;
    TextView distToSenderTextView;

    // for tagging the first quality indicator
    boolean firstChange = true;

    /**
     * we need to pass it the views, whose contents should reflect the current location, distances etc.
     * @param context
     * @param latTextView
     * @param longTextView
     * @param altTextView
     * @param distTextView
     * @param distToSenderTextView
     * @param senderLocation
     */
    public LocationHandler(Context context, TextView latTextView, TextView longTextView, TextView altTextView, TextView distTextView,
                           TextView distToSenderTextView, Location senderLocation) {
        this.context = context;
        this.latTextView = latTextView;
        this.longTextView = longTextView;
        this.altTextView = altTextView;
        this.distTextView = distTextView;
        this.distToSenderTextView = distToSenderTextView;
        this.senderLocation = senderLocation;

        setupGPS();
    }

    @Override
    /**
     * we change the given views when a location change is reported by this callback.
     */
    public void onLocationChanged(Location location) {
        currentLocation = location;

        if(distToSenderTextView != null)
            handleDistToSenderView();

        latTextView.setText("latitude: " + getLatitude());
        longTextView.setText("longitude: " + getLongitude());
        altTextView.setText("altitude: " + getAltitude() + " m");
        if (taggedLocation != null) {
            float distance = location.distanceTo(taggedLocation);
            if (distTextView != null)
                distTextView.setText("distance: " + distance);
        }
    }

    /**
     * In some cases we want to know the distance between the LoRa devices. Or more specifically calculate
     * the distance between previously logged GPS data and the current location.
     */
    private void handleDistToSenderView() {
        if(senderLocation.getLatitude() != -1 && senderLocation.getLongitude() != -1) {
            if(currentLocation != null)
                distToSenderTextView.setText("distance: " + currentLocation.distanceTo(senderLocation));
            else
                distToSenderTextView.setText("no gps data available here yet");
        } else {
            distToSenderTextView.setText("no sender location available yet");
        }
    }

    /**
     * outsourced code block, which sets up the basic functions of this class.
     */
    private void setupGPS() {
        // API access
        locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        try {
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, this);
        } catch (SecurityException e) {
            System.out.println(e);
        }

        // permission we need to ask from the user.
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        // get the first GPS data values
        Location firstLocation = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        currentLocation = firstLocation;

        if(distToSenderTextView != null)
            handleDistToSenderView();

        if (firstLocation != null) {
            latTextView.setText("latitude: " + firstLocation.getLatitude());
            longTextView.setText("longitude: " + firstLocation.getLongitude());
            altTextView.setText("altitude: " + firstLocation.getAltitude()  + " m");
        }
    }

    /**
     * to measure distance in explore mode.
     */
    public void tagLocation() {
        if (currentLocation != null) {
            taggedLocation = currentLocation;
            if (distTextView != null)
                distTextView.setText("distance: 0");
        } else {
            Toast.makeText(context, "No location available for tagging", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * getters for three dimensions of GPS data (the next three methods)
     * @return
     */
    public String getLatitude() {
        if (currentLocation != null)
            return Double.toString(currentLocation.getLatitude());
        else
            return "-1";

    }

    public String getLongitude() {
        if (currentLocation != null)
            return Double.toString(currentLocation.getLongitude());
        else
            return "-1";
    }

    public String getAltitude() {
        if (currentLocation != null)
            return Double.toString(currentLocation.getAltitude());
        else
            return "-1";
    }

    /**
     * need to close the listeners
     */
    public void close() {
        locationManager.removeUpdates(this);
    }

    /* galaxy s5 apparently needs these implemented to avoid an AbstractMethodError*/
    @Override
    public void onStatusChanged(String s, int i, Bundle bundle) {}

    @Override
    public void onProviderEnabled(@NonNull String provider) {}

    @Override
    public void onProviderDisabled(@NonNull String provider) {}
}
