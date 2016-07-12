#include <stdio.h>

/* sensor and component libraries */
//#include <BH1750FVI.h>      // light sensor
#include <DHT.h>            // temperature sensor
#include <LiquidCrystal.h>  // lcd display
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


char* wlanPass0 = "tobiaSCHABERundNADINEseeger";
char* wlanPass1 = "MajorTom";
char* wlanPass2 = "jmca2165";

/* pin definitions */
#define pinDHTsensor  2
#define pinWLANbutton 3

#define pinHasError   5
#define pinStatusLED  BUILTIN_LED

#define pinLCD1       4
#define pinLCD2       6
#define pinLCD3       10
#define pinLCD4       11
#define pinLCD5       12
#define pinLCD6       13

/* sensor components */
//BH1750FVI     lightSensor;
LiquidCrystal lcd(pinLCD1, pinLCD2, pinLCD3, pinLCD4, pinLCD5, pinLCD6);
DHT           dht(pinDHTsensor, DHT22);

WiFiClient    espClient;
PubSubClient  mqttClient(espClient);

/* value-holding variables */
uint16_t      lightIntensity = 0;
uint8_t       currentWlanId  = 0;

/* error indicators */
bool          hasWLANError   = false;
bool          hasMQTTError   = false;


/* ============================================================================================================================
   SETUP method
   ============================================================================================================================ */
void setup() {

  Serial.begin(115200);

  pinMode(pinWLANbutton, INPUT);
  pinMode(pinHasError, OUTPUT);
  pinMode(pinStatusLED, OUTPUT);

  lcd.begin(16, 2);
  dht.begin();
  //lightSensor.begin();

  lcd.clear();
  //LightSensor.SetAddress(Device_Address_L);
  //LightSensor.SetMode(Continuous_H_resolution_Mode);

  mqttClient.setServer(mqttServer, mqttPort);

  /* initial wlan connection */
  switchWLAN(true);


}


/* ============================================================================================================================
   LOOP method
   ============================================================================================================================ */
void loop() {

  /* flash the error LED if there are any errors */
  flashErrorLEDOnErrors();

  /* if "switch wlan button" is pressed */
  if (digitalRead(pinWLANbutton) == HIGH) {

    /* give user feedback to button pushed */
    flashStatusLED();

    switchWLAN(false);
    /* wait until button no longer pushed */
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    connectMQTT();
  } else {
    logMessage("WLAN not connected, thus not trying to connect mqtt!");
  }

  delay(globalDelayMs);

}


/* ============================================================================================================================
   disconnect WLAN and switch to a new WLAN round robin. if initial==true, don't disconnect and connect to default wlan (id 0)
   ============================================================================================================================ */
void switchWLAN(bool initial) {
  logMessage("switching WLAN...");

  hasWLANError = false;

  /* wlanId -1? -> set default wlan (= id 0) */
  if (initial) {
    currentWlanId = 0;
    connectWLAN(wlanSSID0, wlanPass0);
  } else {

    disconnectWLAN();

    /* switch wlan between different modes (currently only 3 modes available!) */
    if (currentWlanId == 0) {
      currentWlanId = 1;
      connectWLAN(wlanSSID1, wlanPass1);
    } else {
      if (currentWlanId == 1) {
        currentWlanId = 2;
        connectWLAN(wlanSSID2, wlanPass2);
      } else {
        currentWlanId = 0;
        connectWLAN(wlanSSID0, wlanPass0);
      }
    }
  }
}


/* ============================================================================================================================
   disconnect WLAN
   ============================================================================================================================ */
void disconnectWLAN() {

  /* try to disconnect if already connected */
  if (WiFi.status() == WL_CONNECTED) {
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
   if status DISCONNECTED, try to connect to the given wlan credentials
   ============================================================================================================================ */
bool connectWLAN(char* ssid, char* pass) {

  logMessage("connecting to wlan: ", false);
  logMessage(ssid);

  if (WiFi.status() != WL_DISCONNECTED) {
    logMessage("ERROR: wlan not in disconnected state!");
    hasWLANError = true;
    return false;
  } else {

    WiFi.begin(ssid, pass);

    int ct = 0;

    while (ct < 10 && WiFi.status() != WL_CONNECTED) {
      delay(500);
      logMessage(".", false);
    }

    if (WiFi.status() != WL_CONNECTED) {
      logMessage("ERROR: could not connect to WLAN");
      hasWLANError = true;
      return false;
    } else {
      logMessage("SUCCESS: connected to WLAN. IP: ", false);
      logMessage(WiFi.localIP().toString());
      return true;
    }

  }
}


/* ============================================================================================================================
   try to connect to the MQTT broker
   ============================================================================================================================ */
bool connectMQTT() {
  logMessage("connecting to MQTT server.");

  int ct = 0;

  /* skip if already connected or too many tries */
  while(ct < 10 && !mqttClient.connected()) {
    mqttClient.connect(clientId);
    
    logMessage(".", false);

    if(mqttClient.connected()) {

      ct=10;
      hasMQTTError = false;
      return true;
    }

    delay(500);    
  }

  logMessage("mqtt connection failed after 10 tries. error:", false);
  logMessage(mqttClient.state());
  
  hasMQTTError = true;
  return false;
}



/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void sendMQTTMessage(String topic, String message) {
  logMessage("sending message...");
  client.publish("outTopic", "hello world");
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
void flashErrorLEDOnErrors() {


  /* if there currently are any known errors, flash LED */
  if (hasWLANError || hasMQTTError) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(pinHasError, LOW);
      delay(100);
      digitalWrite(pinHasError, HIGH);
      delay(100);
    }
  } else {
    digitalWrite(pinHasError, LOW);
  }
}


/* ============================================================================================================================
   ============================================================================================================================
   ============================================================================================================================ */
void flashStatusLED() {

  digitalWrite(pinStatusLED, HIGH);
  delay(50);
  digitalWrite(pinStatusLED, LOW);
  delay(50);
  digitalWrite(pinStatusLED, HIGH);
  delay(50);
  digitalWrite(pinStatusLED, LOW);
  delay(50);
}


