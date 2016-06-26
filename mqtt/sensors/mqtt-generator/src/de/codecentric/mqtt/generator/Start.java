package de.codecentric.mqtt.generator;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by tobias on 22.06.16.
 */
public class Start {

    public static void main(String... args) throws Exception {

        MQTTConnector connector = new MQTTConnector("testtopic", "tcp://localhost:1883");

        connector.connect();


        List<SenderThread> threads = new ArrayList<SenderThread>();

        for(int i=0; i<1; i++) {
            threads.add(new SenderThread(connector));
        }

        for(Thread t : threads) {

            t.start();
        }


        // lauf mal ne Weile bevor du aufhörst.
        Thread.sleep(3000000);

        for(Thread t : threads) {
            t.interrupt();
        }

        connector.disconnect();

    }



}
