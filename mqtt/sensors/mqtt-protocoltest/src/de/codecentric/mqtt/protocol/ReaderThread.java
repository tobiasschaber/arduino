package de.codecentric.mqtt.protocol;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;

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

        byte[] buffer = new byte[200];

        int ct = 0;
        int read;

        try {
            while((is.read(buffer)) != -1) {
                ++ct;
                //System.out.println("reading:" + read);
            }
            System.out.println("FINISHED READING " + ct + " bytes");
            System.out.println("========================================= PAYLOAD:");
            System.out.println(new String(buffer, StandardCharsets.UTF_8));
            System.out.println("========================================= PAYLOAD BYTES:");

            for(int i=0;i<buffer.length;i++) {
                if(buffer[i] != 0)
                System.out.println(Integer.toBinaryString(buffer[i]));

            }


        } catch (Exception e) {
            e.printStackTrace();
        }



    }
}
