#include <BH1750FVI.h>

#include <Ultrasonic.h>

#include <stdio.h>

#include <TaskScheduler.h>

/* sensor and component libraries */

#include <DHT.h>            // temperature sensor
#include <ESP8266WiFi.h>    // wlan module

/* other libraries */
#include <Wire.h>           // I2C library
#include <PubSubClient.h>   // mqtt library

/* global delay for each turnaround */
#define globalDelayMs 5000

/* mqtt server information */
#define mqttHost      "172.17.21.149"
#define mqttPort      (1883)

/* mqtt client id of this device */
#define clientId      "sensorClient"

/* WLAN credentials */
char* wlanSSID0 = "Kfb_Outpost";
char* wlanSSID1 = "codecentric";
char* wlanSSID2 = "AndroidAP";


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

/* sensor components */
BH1750FVI     lightSensor;
DHT           dht(pinDHTsensor, DHT22);  
WiFiClient    espClient;
PubSubClient  mqttClient(espClient);
Ultrasonic    ultrasonic(pinDistTrig, pinDistEcho, 30000); // (Trig PIN,Echo PIN)

/* value-holding variables */
uint16_t      lightIntensity = 0;
uint8_t       currentWlanId  = -1;

/* error indicators */
bool          hasWLANError   = false;
bool          hasMQTTError   = false;

Scheduler runner;

void connectWLANThread();
void connectMQTTThread();
void sendMQTTThread();
void flashErrorLEDOnErrorsThread();

Task connectWLANTask(200, TASK_FOREVER, &connectWLANThread);
Task connectMQTTTask(1000, TASK_FOREVER, &connectMQTTThread);
Task sendMQTTTask(1000, TASK_FOREVER, &sendMQTTThread);
Task flashErrorLEDOnErrorsTask(200, TASK_FOREVER, &flashErrorLEDOnErrorsThread);



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
  
  digitalWrite(pinStatusLED, HIGH);

  dht.begin();
  lightSensor.begin();

  lightSensor.SetAddress(Device_Address_L);
  lightSensor.SetMode(Continuous_H_resolution_Mode);

  mqttClient.setServer(mqttHost, mqttPort);

  runner.init();

  runner.addTask(connectWLANTask);
  runner.addTask(connectMQTTTask);
  runner.addTask(sendMQTTTask);
  runner.addTask(flashErrorLEDOnErrorsTask);

  flashErrorLEDOnErrorsTask.enable();
  connectWLANTask.enable();

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
    flashStatusLED();

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
  } else {
    //logMessage("SUCCESS: connected to WLAN. IP: ", false);
    //logMessage(WiFi.localIP().toString());
    hasWLANError = false;
    connectMQTTTask.enable();
  }

}


/* ============================================================================================================================
   disconnect WLAN
   ============================================================================================================================ */
void disconnectWLAN() {

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
    
    logMessage("sending message...");
    flashStatusLED();
    
    mqttClient.publish(topic, message);
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
   ============================================================================================================================
   ============================================================================================================================ */
void flashStatusLED() {

  digitalWrite(pinStatusLED, LOW);
  delay(50);
  digitalWrite(pinStatusLED, HIGH);
  delay(50);
  digitalWrite(pinStatusLED, LOW);
  delay(50);
  digitalWrite(pinStatusLED, HIGH);
  delay(50);
}


/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void sendMQTTThread() {

  uint16_t lightIntens = lightSensor.GetLightIntensity();
  Serial.print("LIGHT:");
  Serial.println(lightIntens);

  long range = -1;

  /* give 10 tries to get a valid distance value */
  for(int i=0; i<10; i++) {
    range = ultrasonic.Ranging(CM);

      if(range != 0) {
        i = 10;
      }
  }
   
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();


  if (isnan(h) || isnan(t)) {
    Serial.println("SENSOR ERROR");
  } else {

    Serial.print("Temp:   ");
    Serial.print(t);
    Serial.println(" C");
    Serial.print("Feucht: ");
    Serial.print(h);
    Serial.println(" %");
 }

  String s = "";
  s += "{\n";
  
  s += "\"temperatur\" : \"";
  s += t;
  s += "\",\n";

  s += "\"light\" : \"";
  s += lightIntens;
  s += "\",\n";
  
  s += "\"feuchtigkeit\" : \"";
  s += h;
  s += "\",\n";
  
  s += "\"distance\" : \"";
  s += range;
  
  s += "\"\n}";

  const char* msg = s.c_str();
 
  Serial.println(s);
  sendMQTTMessage("weather", msg);

}

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



void disconnectMQTT() {

  logMessage("disconnecting and disabling mqtt");
  sendMQTTTask.disable();
  connectMQTTTask.disable();
  mqttClient.disconnect();

}

