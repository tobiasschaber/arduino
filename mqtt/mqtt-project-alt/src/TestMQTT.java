
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;



/**
 * Created by tobias on 21.06.16.
 */
public class TestMQTT {

        public static void main(String[] args) {

            String topic        = "MQTT Examples";
            String content      = "Message from MqttPublishSample";
            int qos             = 2;
            String broker       = "tcp://localhost:1883";
            String clientId     = "JavaSample";
            MemoryPersistence persistence = new MemoryPersistence();

            try {



                MqttClient client = new MqttClient(broker, clientId, persistence);
                MqttConnectOptions opts = new MqttConnectOptions();
                opts.setCleanSession(true);

                client.connect();

                MqttMessage message = new MqttMessage(content.getBytes());
                message.setQos(qos);
                client.subscribe(topic);
                client.setCallback(new MqttCallback() {
                    @Override
                    public void connectionLost(Throwable throwable) {

                    }

                    @Override
                    public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
                        System.out.println("GOT MESSAGE :) " + mqttMessage.toString());

                    }

                    @Override
                    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {

                    }
                });
                Thread.sleep(1000);
                client.publish(topic, message);
                System.out.println("Message published");

                Thread.sleep(1000);

                System.out.println("AHA");
                client.disconnect();
                System.exit(1);

                MqttClient sampleClient = new MqttClient(broker, clientId, persistence);
                MqttConnectOptions connOpts = new MqttConnectOptions();
                connOpts.setCleanSession(true);
                System.out.println("Connecting to broker: "+broker);
                sampleClient.connect(connOpts);
                System.out.println("Connected");
                System.out.println("Publishing message: "+content);
//                MqttMessage message = new MqttMessage(content.getBytes());
                message.setQos(qos);
                sampleClient.publish(topic, message);
                System.out.println("Message published");
                sampleClient.disconnect();
                System.out.println("Disconnected");
                System.exit(0);
            } catch(MqttException me) {
                System.out.println("reason "+me.getReasonCode());
                System.out.println("msg "+me.getMessage());
                System.out.println("loc "+me.getLocalizedMessage());
                System.out.println("cause "+me.getCause());
                System.out.println("excep "+me);
                me.printStackTrace();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

}
