/*
   SKETCH FOR MY SELFMADE LCD-SHIELD AND MY SELFMADE SENSOR-SHIELD!

   full sketch for measuring temperature, humidity and brightness
   and displaying it on 2 LEDs and one LCD display.
   you can just leave out the two LEDs and only use the LCD display.

*/

// library for light sensor
#include <BH1750FVI.h>
#include <Wire.h> // I2C Library
#include <stdio.h>

// library for wlan module
#include "ESP8266.h"
#include <doxygen.h>

// library for temperature/humidity sensor
#include "DHT.h"

// library for LCD display
#include <LiquidCrystal.h>

// digital pin for temperature/humidity sensor
#define DHTPIN 2
// type of temperature/humidity sensor
#define DHTTYPE DHT22

int pinHeatLED = 9;
int pinNightLED = 5;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 6, 10, 11, 12, 13);
BH1750FVI LightSensor;

// Variable für Lichtintensität
uint16_t Light_Intensity = 0;

int lcdWaitingTime = 1200;

/* define when the heat LED will switch off (min) or has maximum brightness (max) */
int heatLEDmin = 20;
int heatLEDmax = 40;

/* define when the night LED will switch off (min) or has maximum brightness (max) */
int nightLEDmin = 0;
int nightLEDmax = 200;


// anzahl messungen pro durchlauf
int loopCount = 20;

// wartezeit zwischen einzelmessungen
int delayTime = 500;

// zählt die iterationen hoch
int currentIteration = 0;

// modus. 0 = Temperaturanzeige, 1 = LUX-Anzeige
int mode = 0;

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);


// create some WLAN artifacts
#define SSID        "Kfb_Outpost"
#define PASSWORD    "tobiasSCHABERundNADINEseeger"
#define HOST_NAME   "www.baidu.com"
#define HOST_PORT   (80)
ESP8266 wifi(Serial);


void setup() {

  Serial.begin(115200);

  /* setup LCD display */
  lcd.begin(16, 2);
  lcd.clear();

  
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
  if (wifi.joinAP(SSID, PASSWORD)) {
    printOutStatus("OK");
  } else {
    printOutStatus("ERR");
  }

  printOutMessage("WLAN INIT..", "FINISHED INIT!");

  delay(5000);

  currentIteration = 0;

  /* set PIN allocation */
  pinMode(pinHeatLED, OUTPUT);
  pinMode(pinNightLED, OUTPUT);

  /* setup light sensor */
  LightSensor.begin();
  LightSensor.SetAddress(Device_Address_L);
  LightSensor.SetMode(Continuous_H_resolution_Mode);

  /* setup temp+hum sensor */
  dht.begin();

}

void loop() {

  /* collect sensor data phasis */

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  Light_Intensity = LightSensor.GetLightIntensity();

  // calculate which phase to show on the LCD display (Temp/Hum or Lux)
  if (currentIteration == loopCount) {
    currentIteration = 0;

    // switch phasis between "show temperature on LCD" and "show lux on LCD"
    if (mode == 0) {
      mode = 1;
    } else {
      mode = 0;
    }

  }

  currentIteration = currentIteration + 1;

  delay(delayTime);
  lcd.clear();


  // ******************************************************** PHASE 1 : LUX-Messung
  if (mode == 0) {

    lcd.print(Light_Intensity);
    lcd.print(" Lux");
  }

  //  ******************************************************** PHASE 2 : Temperatur/Feuchtigkeits-Messung
  if (mode == 1) {

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      lcd.print("SENSOR ERROR");
    } else {

      lcd.print("Temp:   ");
      lcd.print(t);
      lcd.print(" C");
      lcd.setCursor(0, 2);
      lcd.print("Feucht: ");
      lcd.print(h);
      lcd.print(" %");

    }
  }

  //  ******************************************************** IN ALL PHASES, ADJUST HEAT LED
  int heatLEDStrength = map(t, heatLEDmin, heatLEDmax, 0, 255);

  if (heatLEDStrength < 0) {
    heatLEDStrength = 0;
  }
  if (heatLEDStrength > 254) {
    heatLEDStrength = 254;
  }

  analogWrite(pinHeatLED, heatLEDStrength );


  //  ******************************************************** IN ALL PHASES, ADJUST NIGHT LED
  int ledNightStrength = map(Light_Intensity, nightLEDmin, nightLEDmax, 255, 0);

  // add bounds for extreme values
  if (ledNightStrength < 0) {
    ledNightStrength = 0;
  }
  if (ledNightStrength > 254) {
    ledNightStrength = 254;
  }

  analogWrite(pinNightLED, ledNightStrength);

  //  Serial.print("NIGHT LED: ");
  //  Serial.print(Light_Intensity);
  //  Serial.print(" -> ");
  //  Serial.print(ledNightStrength);
  //  Serial.print(" \n");

  // Serial.print("HEAT LED: ");
  // Serial.print(heatLEDStrength);
  // Serial.print(" \n");




  // debug some infos to the serial monitor
  //Serial.print("Humidity: ");
  //Serial.print(h);
  //Serial.print(" %\t");
  //Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.print(" *C ");
}


void printOutMessage(String headline, String currentDoing) {


  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print(headline);

  lcd.setCursor(0, 1);
  lcd.print(currentDoing);

  Serial.println(headline);
  Serial.println(currentDoing);
  Serial.println("------------------------------------");


  delay(lcdWaitingTime);

}


void printOutStatus(String status) {
  lcd.print("..");
  lcd.print(status);
  delay(lcdWaitingTime-200);

}



void pushValuesToBackend() {

    uint8_t buffer[1024] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    char *hello = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
    wifi.send((const uint8_t*)hello, strlen(hello));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }

    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }

  
}

