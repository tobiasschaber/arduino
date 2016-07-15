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

    public MessageHandler() {

    }

    @Override
    public void connectionLost(Throwable throwable) {
        System.out.println("connection lost!");

    }

    @Override
    public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
        System.out.println("==========================================================");
        System.out.println(mqttMessage.toString());
        System.out.println("==========================================================");

//        try {
//            HttpResponse<String> postResponse = Unirest.post("http://10.0.3.131:9200/mqtt/test/1")
//                    .header("accept", "application/json")
//                    .header("Content-Type", "application/json")
//                    .basicAuth("esadmin", "esadmin")
//                    .body("{\"name\" : \"test\", \"wert\" : 10}").asString();
//            //        .asJson();

//                    System.out.println("---");
//        System.out.println(postResponse.getBody());
//        System.out.println(postResponse.getStatusText());
//        System.out.println(postResponse.getStatus());
//        System.out.println("---");
//        } catch(Exception e) {
//            System.out.println("XX");
//            e.printStackTrace();
//        }



    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {
        System.out.println("message delivered!");

    }
}
