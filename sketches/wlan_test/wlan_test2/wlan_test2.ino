#include <ESP8266.h>
#include <doxygen.h>

//#include <SoftwareSerial.h>


// DOKU ESP8266 LIBRARY: https://github.com/itead/ITEADLIB_Arduino_WeeESP8266


// ####################################
// DAS GEHT AT+CWDHCP=1,1
// ####################################

#define SSIDA       "Kfb_Outpost"
#define PASSWORD    "tobiasSCHABERundNADINEseeger"
#define HOST_NAME   "www.baidu.com"
#define HOST_PORT   (80)

// with software serial
//SoftwareSerial mySerial(0, 1); /* RX:D3, TX:D2 */
//ESP8266 wifi(mySerial);

// without software serial
ESP8266 wifi(Serial);

void setup() {
  Serial.begin(115200);
  Serial.print("setup begin\r\n");
  Serial.print("FW Version: ");
  Serial.println(wifi.getVersion().c_str());
  Serial.println("xx");

    Serial.println("ENABLE DHCP");
    if (wifi.enableClientDHCP(1, true)) {
      Serial.println("OKOKOKOK");
    } else {
      Serial.println("ERRRRRRRRRRRR");
    }

    
    if (wifi.setOprToStation()) {
        Serial.print("to station ok\r\n");
    } else {
        Serial.print("to station err\r\n");
    }

    wifi.enableMUX();
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }

    //wifi.getAPList();
    
    if (wifi.joinAP(SSIDA, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");       
        Serial.println(wifi.getLocalIP().c_str());
    } else {
        Serial.print("Join AP failure\r\n");
    }

    Serial.print("setup end\r\n");


}

void loop() {

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }
    delay(3000);
  // put your main code here, to run repeatedly:

}


