package de.codecentric.mqtt.protocol;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.Charset;

/**
 * Created by tobias on 06.07.16.
 */
public class ProtocolTest {

    public static void main(String...args) throws Exception {



        ServerSocket ssock = new ServerSocket(1883);
        Socket sock;

        for(int i=0; i<10000; i++) {
            System.out.println("waiting for connection");

            sock = ssock.accept();
            System.out.println("nehme an");



            Thread reader = new ReaderThread(sock);

            reader.start();

//            int ct =0;
//
//            while(((x = is.read()) != -1) && ct < 10) {
//                ct++;
//                System.out.println("read: " + x);
//            }
//            System.out.println("stream leer");
//
//            byte[] buffer = new byte[1000];
//            int read;
//
//

//            while((read = bis.read(buffer)) != -1) {
//                System.out.println(read);
//
//            }
//
//            for(int ir=1; ir<buffer.length; ir++) {
//                if(ir % 8 == 0)
//                System.out.println(buffer[ir]);
//                else
//                    System.out.print(buffer[ir]);
//            }





            //osw.write("SEND OK");


        }




    }


}
