/*
  ESP-DASH V2
  Made by Ayush Sharma
  Github URL: https://github.com/ayushsharma82/ESP-DASH
  Support Me: https://www.patreon.com/asrocks5

  - Version Changelog -
  V1.0.0 - 11 Nov. 2017 - Library was Born
  V1.0.1 - 13 Nov. 2017 - Fixed Empty SPIFFS Issue
  V1.0.2 - 13 Nov. 2017 - Improvements on SPIFFS Issue
  V1.0.3 - 24 Dec. 2017 - Pushing to Library Manager

  = Library Rewritten! =
  V2.0.0 - 25 Jan 2019 - Wohoo! A breakthrough in performance and capabilities!

* ****************
  The Automated Farmer 2020 - https://theautomatedfarmer.com

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

#include <ESP8266mDNS.h>        // mDNS let's us set a custom domain
#include <OneWire.h>            // Temperature sensor libraries
#include <DallasTemperature.h>

// To avoid hardcoded WiFi credentials: https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h> 

// Data wire is connected to GPIO 2
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

AsyncWebServer server(80);
DNSServer dns;

float readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return 0.0;
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC); 
  }
  return float(tempC);
}

float readPH(){
  // pH Analog pin
  int ph_pin = A0;  
  // Read Analog pin 
  int measure = analogRead(ph_pin);
  Serial.print("Measure: ");
  Serial.print(measure);

  double voltage = 5 / 1024.0 * measure; //classic digital to voltage conversion
  Serial.print("\tVoltage: ");
  Serial.print(voltage, 3);

  // PH_step = (voltage@PH7 - voltage@PH4) / (PH7 - PH4)
  // PH_probe = PH7 - ((voltage@PH7 - voltage@probe) / PH_step)
  float Po = 7 + ((2.5 - voltage) / 0.77);
  Serial.print("\tPH: ");
  Serial.print(Po, 3);

  return float(Po);
}

void setup() {
  Serial.begin(115200);

  AsyncWiFiManager wifiManager(&server,&dns);

  wifiManager.resetSettings();
  wifiManager.autoConnect("AutoFarm","password"); 
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Now we can open a browser at autofarm.local
  if (!MDNS.begin("autofarm")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance
  // Add Respective Cards
  ESPDash.addNumberCard("num1", "pH", 264);
  ESPDash.addTemperatureCard("temp1", "Water Temperature", 0, 20);

  server.begin();
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {  
  MDNS.update(); 
    
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  ESPDash.updateNumberCard("num1", readPH());
  ESPDash.updateTemperatureCard("temp1", readDSTemperatureC());
  delay(3000);
}
