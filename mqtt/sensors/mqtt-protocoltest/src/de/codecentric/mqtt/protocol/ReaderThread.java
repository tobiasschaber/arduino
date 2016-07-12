package de.codecentric.mqtt.protocol;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

/**
 * Created by tobias on 08.07.16.
 */
public class ReaderThread extends Thread {

    Socket socket;
    InputStream is;
    OutputStream os;


    public ReaderThread(Socket socket) throws Exception {

        this.socket = socket;
        this.is = socket.getInputStream();
        this.os = socket.getOutputStream();

    }

    @Override
    public void run() {

        byte[] buffer = new byte[200];

        int ct = 0;
        int read;

        log("starte empfangen");
        try {
            while((is.read(buffer)) != -1) {
                ++ct;
            }

            log("FINISHED READING " + ct + " bytes");
            log("=========================================== PAYLOAD:");
            log(new String(buffer, StandardCharsets.UTF_8));
            log("=========================================== PAYLOAD BYTES:");

            for(int i=0;i<buffer.length;i++) {
                if(buffer[i] != 0) {
                    if (i % 4 == 0) {
                        System.out.println(getName() + " : " + "");
                    }

                    System.out.print(String.format("%8s", Integer.toBinaryString(buffer[i])).replace(' ', '0')+ " | ");


                }
            }

            log("finished empfangen");


            log("writing response");
            byte[] response = "OK OK OK ".getBytes(Charset.forName("UTF-8"));
            os.write(response);
            //os.flush();


            log("finished writing response");


            Thread.sleep(4000);

            log("closing socket");
            socket.close();


        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public void log(String msg) {
        System.out.println(getName() + " : " + msg);
    }
}
