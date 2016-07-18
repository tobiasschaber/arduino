package de.codecentric.mqtt.receiver;

import com.mashape.unirest.http.HttpResponse;
import com.mashape.unirest.http.Unirest;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by tobias on 15.07.16.
 */
public class RestSender implements Runnable {


    private String msg;

    public RestSender( String msg) {
        this.msg = msg;
    }


    @Override
    public void run() {


       try {

           //yyyy-MM-dd'T'HH:mm:ss.SSSXXX

            DateFormat tsformatter = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSXXX");
            String time = tsformatter.format(new Date().getTime());

           String fullMsg = msg.replace("{\n", "{\n\"@timestamp\" : \"" + time + "\",\n");

            HttpResponse<String> postResponse = Unirest.post("http://10.0.3.131:9200/arduino/info")
                    .header("accept", "application/json")
                    .header("Content-Type", "application/json")
                    .basicAuth("esadmin", "esadmin")
                    .body(fullMsg).asString();

           System.out.println(fullMsg);
            System.out.println("---");
//            System.out.println(postResponse.getBody());
//            System.out.println(postResponse.getStatusText());
            System.out.println(postResponse.getStatus());
            System.out.println("---");
        } catch(Exception e) {
            System.out.println("XX");
            e.printStackTrace();
        }

    }
}
