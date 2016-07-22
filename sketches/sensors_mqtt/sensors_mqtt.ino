

#include <LiquidCrystal.h>


/*
 * IMPORTANT NOTE:
 * 
 * you have to overrwide the file "PubSubClient.h" and set
 * FROM:  #define MQTT_MAX_PACKET_SIZE 128
 * TO:    #define MQTT_MAX_PACKET_SIZE 512
 * 
 * OTHERWISE YOUR MQTT MESSAGES WILL BE DROPPED SILENTLY!
 * 
 */



#include <TaskScheduler.h>

#include <BH1750FVI.h>      // light sensor
#include <Ultrasonic.h>     // distance sensor
#include <DHT.h>            // temperature sensor

/* sensor and component libraries */

#include <ESP8266WiFi.h>    // wlan module
#include <WiFiUdp.h>        // wlan UDP module for NTP time sync
#include <PubSubClient.h>   // mqtt library

/* other libraries */
#include <Wire.h>           // I2C library
#include <TimeLib.h>        // Time management

/* mqtt client id of this device */
#define clientId  "sensorClient"



/* WLAN credentials */
String wlanSSIDs[3] = {"Kfb_Outpost",                 "codecentric",  "AndroidAP"};
String wlanPws[3]   = {"tobiasSCHABERundNADINEseeger","MajorTom",     "jmca2165"};

#define mqttPort      (1883)
#define mqttHost     "52.29.251.255"

/* pin definitions */

#define pinWLANbutton BUILTIN_LED // = D0

#define pinDistTrig   D6
#define pinDistEcho   D8

#define pinLightScl   D3
#define pinLightSda   D4

#define spiClk        D5
#define spiMosi       D7
#define spiLatch      D2   

// TODO: REPLACE THIS IF SPI USED!
#define pinDHTsensor  D1


BH1750FVI     lightSensor;
DHT           dht(pinDHTsensor, DHT22);
WiFiClient    espClient;
WiFiUDP       udp;
PubSubClient  mqttClient(espClient);
Ultrasonic    ultrasonic(pinDistTrig, pinDistEcho, 30000); // (Trig PIN,Echo PIN)

/* value-holding variables */
uint16_t      lightIntensity = 0;
int           currentWlanId  = -1;

/* error indicators */
bool          hasWLANError    = false;
bool          hasMQTTError    = false;
bool          isWlanConnected = false;


static const char ntpServerName[] = "de.pool.ntp.org";
const int timeZone = 2; // Central European Time

/* time management */
const int     NTP_PACKET_SIZE = 48;
byte          packetBuffer[NTP_PACKET_SIZE];
unsigned int  localTimePort   = 2390;

Scheduler runner;

void connectWLANThread();
void connectMQTTThread();
void sendMQTTThread();
void distanceAlarmThread();

Task connectWLANTask(200, TASK_FOREVER, &connectWLANThread);
Task distanceAlarmTask(50, TASK_FOREVER, &distanceAlarmThread);
Task connectMQTTTask(1000, TASK_FOREVER, &connectMQTTThread);
Task sendMQTTTask(1000, TASK_FOREVER, &sendMQTTThread);



void setup() {

  currentWlanId  = -1;

  Serial.begin(115200);
  
  Wire.begin(pinLightSda, pinLightScl);

  Serial.println("BOOTING NOW...");

  pinMode(pinWLANbutton, INPUT);
  //digitalWrite(pinWLANbutton, LOW);
  
  //pinMode(pinStatusLED, OUTPUT);
  pinMode(pinDistEcho, INPUT);
  pinMode(pinDistTrig, OUTPUT);

  dht.begin();
  lightSensor.begin();

  lightSensor.SetAddress(0x23); /* set i2c address. 0x23 means: address pin is pinned to GND!" */
  lightSensor.SetMode(Continuous_H_resolution_Mode);

  runner.init();

  runner.addTask(connectWLANTask);
  runner.addTask(distanceAlarmTask);
  runner.addTask(connectMQTTTask);
  runner.addTask(sendMQTTTask);

  
  connectWLANTask.enable();
  distanceAlarmTask.enable();

}


/* ============================================================================================================================
   LOOP method
   ============================================================================================================================ */
void loop() {

  runner.execute();

}



/* ============================================================================================================================
   scan for available WLAN networks
   ============================================================================================================================ */
int scanWLANNetworks() {
  
  logMessage("scanning for networks...", false);
  int n = WiFi.scanNetworks();
  logMessage("done", true);

   if (n == 0) {
    logMessage("no networks found", false);
    return -1;
   }
  
   for(int i=0; i<n; i++) {
  
    if(WiFi.SSID(i) == wlanSSIDs[0]) {
      logMessage("found wlan: ", false);
      logMessage(WiFi.SSID(i), true);
      return 0;
    }
  
    if(WiFi.SSID(i) == wlanSSIDs[1]) {
      logMessage("found wlan: ", false);
      logMessage(WiFi.SSID(i), true);
      return 1;
    }
  
    if(WiFi.SSID(i) == wlanSSIDs[2]) {
      logMessage("found wlan: ", false);
      logMessage(WiFi.SSID(i), true);
      return 2;
    }
  }

  logMessage("found no known wlan");
  return 0;
}


/* ============================================================================================================================
   disconnect WLAN and switch to a new WLAN round robin. if initial==true, don't disconnect and connect to default wlan (id 0)
   ============================================================================================================================ */
void connectWLANThread() {

  /* set default WLAN credentials */
  String useSSID = wlanSSIDs[0];
  String usePass = wlanPws[0];

  /* if "switch wlan button" is pressed or intial mode */
  if (digitalRead(pinWLANbutton) == HIGH || currentWlanId == -1) {

    logMessage("setting up new wlan connection", true);

    if (WiFi.status() == WL_CONNECTED) {
      disconnectWLAN();
      logMessage("disconnected from current wlan", true);
    }

    currentWlanId = scanWLANNetworks();
  }

  /* select the wlan setting to use */
  useSSID = wlanSSIDs[currentWlanId];
  usePass = wlanPws[currentWlanId];

  if (WiFi.status() == WL_IDLE_STATUS || WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_NO_SSID_AVAIL) {
    logMessage("connecting to wlan: ", false);
    logMessage(useSSID);



    WiFi.begin(useSSID.c_str(), usePass.c_str());
  }


  if (WiFi.status() != WL_CONNECTED) {
    hasWLANError = true;
    isWlanConnected = false;
  } else {

    /* recognize first run after successfull connection */
    
    if(!isWlanConnected) {

      isWlanConnected = true;

      logMessage("SUCCESS: connected to WLAN. IP: ", false);
      logMessage(WiFi.localIP().toString());

      Serial.println("waiting for time sync");
      udp.begin(localTimePort);
      delay(100);
      setSyncProvider(getNtpTime);
      setSyncInterval(86400);

      while(timeStatus()== timeNotSet) {
        getNtpTime();
        delay(10); 
      }

      Serial.println("finished");
      udp.stop();
    }

    
   
    hasWLANError = false;
    connectMQTTTask.enable();

    
  }

}




/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void logMessage(String msg, bool nl) {
  if (nl) {
    Serial.println(msg);
  } else {
    Serial.print(msg);
  }
}

/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void logMessage(String msg) {
  logMessage(msg, true);
}




/* ============================================================================================================================
   check the NTP time server for new time info (not implemented by me)
   ============================================================================================================================ */
time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request...");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}



/* ============================================================================================================================
   send the get NTP time request (not implemented by me)
   ============================================================================================================================ */
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}





/* ============================================================================================================================
   try to connect to the MQTT broker
   ============================================================================================================================ */
void connectMQTTThread() {

  /* skip if already connected or too many tries */
  if (!mqttClient.connected()) {

    logMessage("connecting to MQTT server...");

    mqttClient.setServer(mqttHost, mqttPort);

    mqttClient.connect(clientId);

    if (mqttClient.connected()) {

      logMessage("successfully connected to mqtt", true);
      sendMQTTTask.enable();
      hasMQTTError = false;
      return;
    }   else {
      logMessage("could not connect to mqtt...", true);
      hasMQTTError = true;
    }
  } else {

    hasMQTTError = false;
  }
}


/* ============================================================================================================================
   thread which sends an mqtt message with the current sensor data
   ============================================================================================================================ */
void sendMQTTThread() {

  String statusMsg = "OK";
  long range = -1;

  uint16_t lightIntens = lightSensor.GetLightIntensity();
  

  /* give 10 tries to get a valid distance value */
  for(int i=0; i<10; i++) {
    range = ultrasonic.Ranging(CM);

      if(range != 0) {
        i = 10;
      }
  }
   
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    h = -1;
    t = -1;
    statusMsg = "sensor error: DHT";
  }

  String s = "";
  s += "{\n";

  s += "\"@timestamp\" : \"";
  s += getCurrentDateTimeString();
  s += "\",\n";
  
  s += "\"temperature\" : ";
  s += t;
  s += ",\n";

  s += "\"luminance\" : ";
  s += lightIntens;
  s += ",\n";
  
  s += "\"humidity\" : ";
  s += h;
  s += ",\n";
  
  s += "\"distance\" : ";
  s += range;
  s += ",\n";

  s += "\"statusMessage\" : \"";
  s += statusMsg;
  s += "\"";
  
  s += "\n}";

  const char* msg = s.c_str();
 
  Serial.println(s);
  sendMQTTMessage("weather", msg);

}





/* ============================================================================================================================
   return the current datetime string based on the NTP time.
   format: yyyy-MM-dd'T'HH:mm:ss.SSSXXX
   ============================================================================================================================ */
String getCurrentDateTimeString() {

  //yyyy-MM-dd'T'HH:mm:ss.SSSXXX
  //json.append(" \"@timestamp\": \"").append(tsformatter.format(current)).append("\",");
  
  String dt = "";
  dt += year();
  dt += "-";
  if(month() < 10) {
    dt += "0";
  }
  dt += month();
  dt += "-";
  if(day() < 10) {
    dt += "0";
  }
  dt += day();
  dt += "T";
  if(hour() < 10) {
    dt += "0";
  }
  dt += hour();
  dt += ":";
  if(minute() < 10) {
    dt += "0";
  }
  dt += minute();
  dt += ":";
  if(second() < 10) {
    dt += "0";
  }
  dt += second();
  dt += ".000+02:00";

  return dt;
}


/* ============================================================================================================================
   disconnect from the mqtt server
   ============================================================================================================================ */
void disconnectMQTT() {

  logMessage("disconnecting and disabling mqtt");
  sendMQTTTask.disable();
  connectMQTTTask.disable();
  mqttClient.disconnect();

}


/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void sendMQTTMessage(char* topic, const char* message) {
  if (mqttClient.connected()) {

    logMessage("== sending message... ================================");

    
    bool msgOk = mqttClient.publish(topic, message);

    if(msgOk != true) {
      Serial.println("error sending message. message too long? see MQTT_MAX_PACKET_SIZE in PubSubClient.h!");
    }
    
  } else {
    logMessage("not connected, can not send message via mqtt", true);
  }
}



/* ============================================================================================================================
   disconnect WLAN
   ============================================================================================================================ */
void disconnectWLAN() {

  isWlanConnected = false;

  /* try to disconnect if already connected */
  if (WiFi.status() == WL_CONNECTED) {
    disconnectMQTT();

    logMessage("disconnecting from wlan");
    WiFi.disconnect();

    /* try to disconnect */
    for (int ct = 0; ct < 100; ct++) {
      if (WiFi.status() != WL_DISCONNECTED) {
        delay(100);
      } else {
        ct = 100;
      }
    }

    isWlanConnected = false;
    
  }
}




/* ============================================================================================================================
   alarm thread which checks if there is a movement in the distance
   ============================================================================================================================ */
void distanceAlarmThread() {

  long  range = ultrasonic.Ranging(CM);

  if(range < 100) {
    String statusMsg = "movement detected!";
  
  
    String s = "";
    s += "{\n";
  
    s += "\"@timestamp\" : \"";
    s += getCurrentDateTimeString();
    s += "\",\n";
    
    s += "\"distance\" : ";
    s += range;
    s += ",\n";
  
    s += "\"statusMessage\" : \"";
    s += statusMsg;
    s += "\"";
    
    s += "\n}";
  
    const char* msg = s.c_str();
   
    Serial.println(s);
    sendMQTTMessage("distance", msg);

  } else {

    // RESET ALARM HERE

  }
  
}
