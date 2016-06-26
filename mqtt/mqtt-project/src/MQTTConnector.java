import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

/**
 * Created by tobias on 22.06.16.
 */
public class MQTTConnector {


    private String topic;
    private String broker;
    private final String clientId = "mqttSender";
    private MemoryPersistence persistence;
    private MqttClient client;
    private MqttConnectOptions opts;

    int qos             = 2;

    public MQTTConnector(String topic, String broker) throws Exception {

        this.topic = topic;
        this.broker = broker;

        persistence = new MemoryPersistence();

        client = new MqttClient(broker, clientId, persistence);
        opts = new MqttConnectOptions();
        opts.setCleanSession(true);
    }


    public void connect() throws  Exception {
        client.connect();

    }

    public void disconnect() throws Exception {
        System.out.println("DIsconnecting");
        client.disconnect();
    }

    public void sendMessage(String message, int qos) throws Exception
    {
        MqttMessage mMessage = new MqttMessage(message.getBytes());
        mMessage.setQos(qos);

        client.publish(topic, mMessage);
    }
}
