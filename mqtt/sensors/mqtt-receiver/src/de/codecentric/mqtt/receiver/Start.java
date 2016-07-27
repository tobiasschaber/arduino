package de.codecentric.mqtt.receiver;

/**
 * Created by tobias on 24.06.16.
 */
public class Start {


    public static void main(String... args) throws Exception {

        MQTTConnector weatherConnector = new MQTTConnector("weather", "tcp://localhost:1883", "weather");
        MQTTConnector distanceConnector = new MQTTConnector("distance", "tcp://localhost:1883", "distance");
        MessageHandler weatherHandler = new MessageHandler("weather");
        MessageHandler distanceHandler = new MessageHandler("distance");

        weatherConnector.connect();

        distanceConnector.connect();


        weatherConnector.registerHandler(weatherHandler);
        distanceConnector.registerHandler(distanceHandler);


        Thread.sleep(30000);


    }

}
