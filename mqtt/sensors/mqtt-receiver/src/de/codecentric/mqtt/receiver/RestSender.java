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


           // ssh -i elkweather.pem ec2-user@ec2-52-29-251-255.eu-central-1.compute.amazonaws.com

            HttpResponse<String> postResponse = Unirest.post("https://search-cc-ka-sensors-elk-qxbxpbn4xrjqfax47oujso7b7i.eu-central-1.es.amazonaws.com/weather/info")
                    .header("accept", "application/json")
                    .header("Content-Type", "application/json")
                    //.basicAuth("esadmin", "esadmin")
                    .body(msg).asString();

           System.out.println(msg);
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
