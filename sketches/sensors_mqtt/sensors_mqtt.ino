

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



/* WLAN and MQTT configuration */
// default is 128 which is not sufficient
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
#endif

/* sensor and component libraries */

#include <ESP8266WiFi.h>    // wlan module
#include <WiFiUdp.h>
#include <PubSubClient.h>   // mqtt library

/* other libraries */
#include <Wire.h>           // I2C library
#include <TimeLib.h>        // Time management

/* mqtt client id of this device */
#define clientId  "sensorClient"

#define mqttPort      (1883)

/* WLAN credentials */
char* wlanSSID0 = "Kfb_Outpost";
char* wlanSSID1 = "codecentric";
char* wlanSSID2 = "AndroidAP";

//#define mqttHost0      "192.168.0.13"
//#define mqttHost1      "172.17.21.149"
//#define mqttHost2      "172.17.21.149"

#define mqttHost0      "52.29.251.255"
#define mqttHost1      "52.29.251.255"
#define mqttHost2      "52.29.251.255"


char* wlanPass0 = "tobiasSCHABERundNADINEseeger";
char* wlanPass1 = "MajorTom";
char* wlanPass2 = "jmca2165";

/* pin definitions */
#define pinDHTsensor  D5
#define pinWLANbutton D3

#define pinHasError   D6
#define pinStatusLED  BUILTIN_LED

#define pinDistEcho   D8
#define pinDistTrig   D7

#define pinLightScl   D1
#define pinLightSda   D2

#define pinAlarmSound D4

/* sensor components */
BH1750FVI     lightSensor;
DHT           dht(pinDHTsensor, DHT22);  
WiFiClient    espClient;
WiFiUDP       udp;
PubSubClient  mqttClient(espClient);
Ultrasonic    ultrasonic(pinDistTrig, pinDistEcho, 30000); // (Trig PIN,Echo PIN)

/* value-holding variables */
uint16_t      lightIntensity = 0;
uint8_t       currentWlanId  = -1;

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
void flashErrorLEDOnErrorsThread();
void distanceAlarmThread();

Task connectWLANTask(200, TASK_FOREVER, &connectWLANThread);
Task connectMQTTTask(1000, TASK_FOREVER, &connectMQTTThread);
Task sendMQTTTask(1000, TASK_FOREVER, &sendMQTTThread);
Task flashErrorLEDOnErrorsTask(200, TASK_FOREVER, &flashErrorLEDOnErrorsThread);
Task distanceAlarmTask(50, TASK_FOREVER, &distanceAlarmThread);



/* ============================================================================================================================
   SETUP method
   ============================================================================================================================ */
void setup() {
  Serial.begin(115200);

  Serial.println("BOOTING NOW...");

  pinMode(pinWLANbutton, INPUT);
  pinMode(pinHasError, OUTPUT);
  pinMode(pinStatusLED, OUTPUT);
  pinMode(pinDistEcho, INPUT);
  pinMode(pinDistTrig, OUTPUT);
  pinMode(pinAlarmSound, OUTPUT);
  
  digitalWrite(pinStatusLED, HIGH);


  dht.begin();
  lightSensor.begin();

  lightSensor.SetAddress(Device_Address_L);
  lightSensor.SetMode(Continuous_H_resolution_Mode);

  runner.init();

  runner.addTask(connectWLANTask);
  runner.addTask(connectMQTTTask);
  runner.addTask(sendMQTTTask);
  runner.addTask(flashErrorLEDOnErrorsTask);
  runner.addTask(distanceAlarmTask);

  flashErrorLEDOnErrorsTask.enable();
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
   disconnect WLAN and switch to a new WLAN round robin. if initial==true, don't disconnect and connect to default wlan (id 0)
   ============================================================================================================================ */
void connectWLANThread() {

  /* set default WLAN credentials */
  char* useSSID = wlanSSID0;
  char* usePass = wlanPass0;

  /* if "switch wlan button" is pressed or intial mode */
  if (digitalRead(pinWLANbutton) == HIGH || currentWlanId == -1) {

    /* give user feedback to button pushed */
    flashStatusLED(4);

    logMessage("setting up new wlan connection", true);
    
    if (WiFi.status() == WL_CONNECTED) {
      disconnectWLAN();
    }

    /* rotate through all available wlan settings */
    if (currentWlanId == 0) {
      currentWlanId = 1;
    } else {
      if (currentWlanId == 1) {
        currentWlanId = 2;
      } else {
        currentWlanId = 0;
      }
    }
  }

  /* select the wlan setting to use */
  if (currentWlanId == 0) {
    useSSID = wlanSSID0;
    usePass = wlanPass0;
  } else {
    if (currentWlanId == 1) {
      useSSID = wlanSSID1;
      usePass = wlanPass1;
    } else {
      useSSID = wlanSSID2;
      usePass = wlanPass2;
    }
  }

  if (WiFi.status() == WL_IDLE_STATUS || WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_NO_SSID_AVAIL) {
    logMessage("connecting to wlan: ", false);
    logMessage(useSSID);

    WiFi.begin(useSSID, usePass);
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
      setSyncInterval(600);

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
  }
}



/* ============================================================================================================================
   try to connect to the MQTT broker
   ============================================================================================================================ */
void connectMQTTThread() {

  /* skip if already connected or too many tries */
  if (!mqttClient.connected()) {

    logMessage("connecting to MQTT server...");

    char* mqttHost;

    /* select the wlan setting to use */
    if (currentWlanId == 0) {
      mqttHost = mqttHost0;
    } else {
      if (currentWlanId == 1) {
        mqttHost = mqttHost1;
      } else {
        mqttHost = mqttHost2;
      }
    }

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
   ============================================================================================================================
   ============================================================================================================================ */
void sendMQTTMessage(char* topic, const char* message) {
  if (mqttClient.connected()) {

    logMessage("== sending message... ================================");
    flashStatusLED(1);

    
    bool msgOk = mqttClient.publish(topic, message);

    if(msgOk != true) {
      Serial.println("error sending message. message too long? see MQTT_MAX_PACKET_SIZE in PubSubClient.h!");
    }
    
  } else {
    logMessage("not connected, can not send message via mqtt", true);
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
   simply flashes the status LED ct times.
   ============================================================================================================================ */
void flashStatusLED(int ct) {

  for(int i=0; i<ct; i++) {
    digitalWrite(pinStatusLED, LOW);
    delay(50);
    digitalWrite(pinStatusLED, HIGH);
    delay(50);    
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
   simply flash the error LEDs
   ============================================================================================================================ */
void flashErrorLEDOnErrorsThread() {

  /* if there currently are any known errors, flash LED */
  if (hasWLANError || hasMQTTError) {

    digitalWrite(pinHasError, LOW);
    delay(100);
    digitalWrite(pinHasError, HIGH);
    delay(100);

  } else {
    digitalWrite(pinHasError, LOW);
  }

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
   alarm thread which checks if there is a movement in the distance
   ============================================================================================================================ */
void distanceAlarmThread() {

  long  range = ultrasonic.Ranging(CM);

  if(range < 100) {

    // TODO: ALARM ACTION HERE

  } else {

    // RESET ALARM HERE

  }
  
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
  while (millis() - beginWait < 2500) {
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

