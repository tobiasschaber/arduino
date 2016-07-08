package de.codecentric.mqtt.protocol;

import java.io.InputStream;

/**
 * Created by tobias on 08.07.16.
 */
public class ReaderThread implements Runnable {

    InputStream is;


    public ReaderThread(InputStream is) {
        this.is = is;

    }

    @Override
    public void run() {

        int ct = 0;
        int read;

        try {
            while((read = is.read()) != -1) {
                ++ct;
                System.out.println("reading:" + read);
            }
            System.out.println("FINISHED READING, STREAM ENDE WITH: " + ct);
        } catch (Exception e) {
            e.printStackTrace();
        }



    }
}
