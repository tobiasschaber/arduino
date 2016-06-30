#include "ESP8266.h"
//#include <SoftwareSerial.h>


#include <doxygen.h>


// DOKU ESP8266 LIBRARY: https://github.com/itead/ITEADLIB_Arduino_WeeESP8266



#define SSIDA       "codecentric"
#define PASSWORD    "MajorTom"
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



void printOutMessage(String message) {
  
}

