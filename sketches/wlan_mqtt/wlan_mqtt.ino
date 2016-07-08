#include <stdio.h>

// library for wlan module
#include "ESP8266.h"
#include <doxygen.h>

// WLAN credentials
#define wlanSSID      "codecentric"
#define wlanPass      "MajorTom"

#include <PubSubClient.h>


// hold IP to show it sometimes
String myip;


// Initialize WIFI module
ESP8266 wifi(Serial);

IPAddress server(172, 16, 0, 2);



void setup() {

  Serial.begin(115200);

  printOutMessage("WLAN INIT..", "station?");
  if (wifi.setOprToStation()) {
    Serial.print("to station ok\r\n");
    printOutStatus("OK");
  } else {
    printOutStatus("ERR");
  }

  printOutMessage("WLAN INIT..", "mux?");
  wifi.enableMUX();
  if (wifi.disableMUX()) {
    printOutStatus("OK");
  } else {
    printOutStatus("ERR");
  }

  printOutMessage("WLAN INIT..", "DHCP?");
  if (wifi.enableClientDHCP(1, true)) {
    printOutStatus("OK");
  } else {
    printOutStatus("ERR");
  }

  printOutMessage("WLAN INIT..", "Join WLAN");
  if (wifi.joinAP(wlanSSID, wlanPass)) {
    printOutStatus("OK");
  } else {
    printOutStatus("ERR");
  }

  /* extract only the IP from the get-IP-message */
  String ip = wifi.getLocalIP().c_str();
  Serial.println("-----------------------------------start");
  Serial.println(ip);
  Serial.println("-----------------------------------then");
  String ip2 = ip.substring(14);
  
  myip = ip2.substring(0, ip2.indexOf('"'));


  client.setServer(server, 1883);
}








void loop() {


  
}











void printOutMessage(String headline, String currentDoing) {

  Serial.println(headline);
  Serial.println(currentDoing);
  Serial.println("------------------------------------");
  
}


void printOutStatus(String status) {
  Serial.println(status);

}


