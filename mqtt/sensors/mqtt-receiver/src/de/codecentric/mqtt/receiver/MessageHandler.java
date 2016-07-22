package de.codecentric.mqtt.receiver;

import com.mashape.unirest.http.HttpResponse;
import com.mashape.unirest.http.JsonNode;
import com.mashape.unirest.http.Unirest;
import com.sun.org.apache.xpath.internal.SourceTree;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.util.UUID;

/**
 * Created by tobias on 24.06.16.
 */
public class MessageHandler implements MqttCallback {

    private String topic;

    public MessageHandler(String topic) {
        this.topic = topic;
    }

    @Override
    public void connectionLost(Throwable throwable) {
        System.out.println("connection lost!");

    }

    @Override
    public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
        String msg = mqttMessage.toString();
        System.out.println("==========================================================");
//        System.out.println(msg);
//        System.out.println("==========================================================");

        RestSender rs = new RestSender(msg, topic);
        new Thread(rs).start();

    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {
        System.out.println("message delivered!");

    }
}
