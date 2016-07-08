
#include "ESP8266.h"
#include "ESP8266MQTT.h"

// WLAN credentials
#define wlanSSID        "codecentric"
#define wlanPass        "MajorTom"
#define mqttServerName  "172.17.21.149"
#define mqttServerPort  (1883)

String myip;


ESP8266 wifi(Serial);
ESP8266MQTT mqtt;


/* ======================================================================================================================== */
/* SETUP */
/* ======================================================================================================================== */
void setup() {
  Serial.begin(115200);

  if (wifi.setOprToStation()) {
    Serial.println("to station: OK");
  } else {
    Serial.println("to station: ERROR");
  }

  wifi.enableMUX();
  if (wifi.disableMUX()) {
    Serial.println("disable mux: OK");
  } else {
    Serial.println("disable mux: ERROR");
  }

 
  if (wifi.enableClientDHCP(1, true)) {
    Serial.println("enable dhcp: OK");
  } else {
    Serial.println("enable dhcp: ERROR");
  }

  
  if (wifi.joinAP(wlanSSID, wlanPass)) {
    Serial.println("join wlan: OK");
  } else {
    Serial.println("join wlan: ERROR");
  }




}

/* ======================================================================================================================== */
/* LOOP */
/* ======================================================================================================================== */
void loop() {

  if(myip == "") {  
  
  
    for(int i=0; i<10; i++) {
      Serial.print(".");
      
      /* extract only the IP from the get-IP-message */
      String ip = wifi.getLocalIP().c_str();
      String ip2 = ip.substring(14);
      
      myip = ip2.substring(0, ip2.indexOf('"'));
      
      if(myip != "") {
        Serial.println("Deine IP: ");
        Serial.println(myip);
        i=10;
      }
    
      delay(2000);
    }

  }


 delay(2000);
 Serial.println("========================================== STARTE SENDEVORGANG");


 wifi.createTCP(mqttServerName, mqttServerPort);
 char *hello = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
 wifi.send((const uint8_t*)hello, strlen(hello));
 wifi.releaseTCP();

Serial.println("ENDE");

  


 
 delay(5000);





}


void dada() {
  
  uint8_t buffer[1024] = {0};
  uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
      Serial.print("Received:[");
      for(uint32_t i = 0; i < len; i++) {
          Serial.print((char)buffer[i]);
      }
      Serial.print("]\r\n");
  }
 
}





