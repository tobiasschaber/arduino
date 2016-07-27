#include <ESP8266MQTT.h>


#include "ESP8266.h"

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

  connectWLAN();

  delay(1000);

}


void connectWLAN() {

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

  /* reset value of myip */
  myip = "";


  /* wait until we have an IP address */
  for(int i=0; i<10; i++) {
    
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

/* ======================================================================================================================== */
/* LOOP */
/* ======================================================================================================================== */
void loop() {
  
  Serial.println("========================================== STARTE SCHREIBVORGANG");

  mqttConnect("testid");

  
Serial.println("X");
delay(5000);
  //dada();
Serial.println("Y");
  

  mqttDisconnect();

Serial.println("Z");
}


/* =======================================================================
 *  connect to MQTT -> create TCP connection, send CONNECT und wait for CONNACK message
 * ======================================================================= */
bool mqttConnect(char* id) {

  bool exas = wifi.createTCP(mqttServerName, mqttServerPort);

  char* conMsg = reinterpret_cast<char*>(
    mqtt.getConnectMessage("testid")
  );
  

  /* send CONNECT */
  wifi.send( mqtt.getConnectMessage("testid"), strlen(conMsg));
  
 /* read CONNACK */

return true;

}


/* =======================================================================
 *  disconnect from mqtt
 * ======================================================================= */
void mqttDisconnect() {
  wifi.releaseTCP();
}



void dada() {
  
  uint8_t buffer[1024] = {0};
  uint32_t len = wifi.recv(buffer, sizeof(buffer), 5000);
  

  
  
  if (len > 0) {
      Serial.print("Received:[");
      for(uint32_t i = 0; i < len; i++) {
          //Serial.print((char)buffer[i]);
      }
      Serial.print("]\r\n");
  }

  Serial.print("finsh write");
 
}





