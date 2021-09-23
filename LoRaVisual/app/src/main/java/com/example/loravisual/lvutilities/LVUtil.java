package com.example.loravisual.lvutilities;

import android.location.Location;

/**
 * bundle of utility methods used throughout the project.
 */
public class LVUtil {

    /**
     * The RSSI value is first stored in a signed int16 at the LoRa device. We negate the negative
     * value and put it into an unsigned int8 (negate it to make it fit in that uint8). Now the size
     * of one byte, we can transmit it to the phone, where we need to get the original value. But the
     * primitive type "byte" in Java is always signed. In this method we get the unsigned value (as an int)
     * which we negate again (at a different place).
     * @param b
     * @return unsigned value
     */
    public static int makeByteUnsigned(byte b) {
        int i = b & 0xff;
        return i;
    }

    /**
     * for debugging and testing only.
     * @param slatitude
     * @param slongitude
     * @param rlatitude
     * @param rlongitude
     */
    public static void getDistance(double slatitude, double slongitude, double rlatitude, double rlongitude) {
        Location slocation = new Location("");
        slocation.setLatitude(slatitude);
        slocation.setLongitude(slongitude);

        Location rlocation = new Location("");
        rlocation.setLatitude(rlatitude);
        rlocation.setLongitude(rlongitude);

        System.out.println("distance: " + slocation.distanceTo(rlocation));

    }
}
