package de.codecentric.mqtt.receiver;

/**
 * Created by tobias on 24.06.16.
 */
public class Start {


    public static void main(String... args) throws Exception {

        MQTTConnector connector = new MQTTConnector("testtopic", "tcp://localhost:1883");
        MessageHandler handler = new MessageHandler();
        connector.connect();


        connector.registerHandler(handler);


        Thread.sleep(30000);


    }

}
