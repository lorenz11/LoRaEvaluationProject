package com.example.loravisual.async;

import android.content.Context;
import android.widget.Toast;

import com.example.loravisual.database.DatabaseHandler;
import com.example.loravisual.lvutilities.FirebaseHandler;

import org.jetbrains.annotations.NotNull;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * This class contains code, that is supposed to be executed on a separate thread, so as not to block the UI thread
 */
public class Client {

    /**
     * as the download of weather data and the upload of experiment data is done in subsequent steps,
     * this method seemingly also does both. (actually the method downloads the weather data, and the
     * callback, that defines the behaviour in case of success, triggers the upload to Firebase.)
     * @param url
     * @param experimentNumber
     * @param context
     * @throws IOException
     */
    public void getWeather(String url, int experimentNumber, Context context) throws IOException {
        DatabaseHandler databaseHandler = new DatabaseHandler(context);
        OkHttpClient client = new OkHttpClient();

        Request request = new Request.Builder()
                .url(url)
                .build();

        client.newCall(request).execute();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(@NotNull okhttp3.Call call, @NotNull IOException e) {

            }

            @Override
            public void onResponse(@NotNull Call call, @NotNull Response response) throws IOException {
                if (!response.isSuccessful()) {
                    throw new IOException("Unexpected code " + response);
                } else {
                    String res = response.body().string();
                    System.out.println(res);
                    try {
                        JSONObject jObject = new JSONObject(res);
                        databaseHandler.setValueInExpInfo(jObject.getJSONObject("current").getString("temp"), "temperature_values", experimentNumber);
                        databaseHandler.setValueInExpInfo(jObject.getJSONObject("current").getString("humidity"), "humidity_values", experimentNumber);
                        databaseHandler.setValueInExpInfo(jObject.getJSONObject("current").getString("pressure"), "pressure_values", experimentNumber);
                        databaseHandler.setValueInExpInfo(jObject.getJSONObject("current")
                                .getJSONArray("weather").getJSONObject(0).getString("description"), "weather_descriptions", experimentNumber);

                        databaseHandler.makeExpInfoText(experimentNumber);

                        new FirebaseHandler(context).uploadData("/storage/emulated/0/Android/data/com.example.loravisual/files/experiment" + experimentNumber + "/experiment_info" + experimentNumber + ".txt",
                                "/experiment" + experimentNumber + "/experiment_info" + experimentNumber + ".txt", "experiment information");
                        new FirebaseHandler(context).uploadData("/storage/emulated/0/Android/data/com.example.loravisual/files/experiment" + experimentNumber + "/experiment_data" + experimentNumber + ".csv",
                                "/experiment" + experimentNumber + "/experiment_data" + experimentNumber + ".csv", "experiment data");
                        response.body().close();
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }
}
