/**
 * Created by tobias on 22.06.16.
 */
public class SenderThread extends Thread {

    private MQTTConnector connector;

    public SenderThread(MQTTConnector connector) {
        this.connector = connector;
    }

    public void run() {

        while(!isInterrupted()) {
            System.out.println("Sending message..");

            try {
                connector.sendMessage("TestMessage" + Math.random(), 1);
                Thread.sleep(1000);
            } catch(InterruptedException ie) {
                return;

            } catch(Exception e) {
                System.out.println("Error sending message...");
            }
        }
    }
}
