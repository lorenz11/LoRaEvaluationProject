package com.example.loravisual.lvutilities;

/**
 * encodes parameter byte. (not really necessary to put in its own class)
 */
public class Encoder {

    /**
     * gets an array of booleans, that represents the LoRa parameter values of one
     * parameter, that is supposed to be tested.
     * @param configs
     * @return
     */
    public static byte encodeParamByte(boolean[] configs) {
        byte confByte = 0;
        for(int i = 0; i < configs.length; i++) {
            if (configs[i]) {
                confByte = (byte) (confByte | (1 << i));
            }
        }
        return confByte;
    }
}
