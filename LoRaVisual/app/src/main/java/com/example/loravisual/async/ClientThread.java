package com.example.loravisual.async;

import android.content.Context;
import android.net.ConnectivityManager;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;

/**
 * this class starts a separate thread. In its run method we define the actions,
 * we want to execute asynchronously.
 */
public class ClientThread extends Thread {
    public static final int GET_WEATHER = 1;

    // any argument that may be needed for a method call in the Client class
    String[] args;
    private int action;
    private Context context;

    public ClientThread(int action, String[] args, Context context) {
        this.action = action;
        this.args = args;
        this.context = context;
        if(!isConnectedToInternet()) {
            Toast.makeText(context, "Not connected to the internet", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void run() {
        Client client = new Client();

        if (action == GET_WEATHER) {
            try {
                client.getWeather(args[0], Integer.parseInt(args[1]), context);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * check whether the phone is connected to the internet.
     * @return
     */
    private boolean isConnectedToInternet() {
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        return cm.getActiveNetworkInfo() != null && cm.getActiveNetworkInfo().isConnected();
    }
}
