import java.util.ArrayList;
import java.util.List;

/**
 * Created by tobias on 22.06.16.
 */
public class Start {

    public static void main(String[] args) throws Exception {

        MQTTConnector connector = new MQTTConnector("testtopic", "tcp://localhost:1883");

        connector.connect();


        List<SenderThread> threads = new ArrayList<SenderThread>();

        for(int i=0; i<10; i++) {
            threads.add(new SenderThread(connector));
        }

        for(Thread t : threads) {

            t.start();
        }


        Thread.sleep(10000);

        for(Thread t : threads) {
            t.interrupt();
        }

        connector.disconnect();

    }



}
