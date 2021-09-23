package com.example.loravisual.lvutilities;

import android.os.CountDownTimer;
import android.view.View;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * This abstract class provides a display of a countdown.
 * Used in @StartExperimentScreen for the delay countdown and the
 * experiment duration countdown. And used in @SenderExploreScreen
 * for the loop functionality.
 */
public abstract class GUICountDownTimer extends CountDownTimer {
    int hoursD, minutesD, secondsD;
    String remainingTime;
    TextView view;
    private int ticks = 1;

    public GUICountDownTimer(long millisInFuture, long countDownInterval, int hours, int minutes, int seconds, TextView view) {
        super(millisInFuture, countDownInterval);
        this.hoursD = hours;
        this.minutesD = minutes;
        this.secondsD = seconds;
        this.view = view;
    }

    @Override
    /**
     * implements the features of a clock.
     */
    public void onTick(long millisUntilFinished) {
        remainingTime = hoursD < 10 ? "0" + hoursD + " : " : Integer.toString(hoursD) + " : ";
        remainingTime += minutesD < 10 ? "0" + minutesD + " : " : Integer.toString(minutesD) + " : ";
        remainingTime += secondsD < 10 ? "0" + secondsD : Integer.toString(secondsD);

        view.setText(remainingTime);

        if (secondsD == 0 && minutesD == 0 && hoursD > 0) {
            secondsD = 59;
            minutesD = 59;
            hoursD--;
        } else if (secondsD == 0 && minutesD > 0) {
            secondsD = 59;
            minutesD--;
        } else {
            secondsD--;
        }

        ticks++;
    }

    public int getTicks() {
        return ticks;
    }
}
