/*
  ESP-DASH V2
  Github URL: https://github.com/ayushsharma82/ESP-DASH

* ****************
  The Automated Farmer 2020 - https://theautomatedfarmer.com

*/
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

#include <ESP8266mDNS.h>        // mDNS let's us set a custom domain
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2     // NodeMCU Pin D4
#define DHTTYPE    DHT11     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// To avoid hardcoded WiFi credentials: https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h> 


AsyncWebServer server(80);
DNSServer dns;

// Get temperature event and print its value.
float readDHTemperatureC() {
  // set placeholder value
  float tempC = 0.0;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    float tempC = event.temperature;
  }
  return float(tempC);
}


void setup() {
  Serial.begin(115200);

  // Initialize the sensor
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  AsyncWiFiManager wifiManager(&server,&dns);

//  wifiManager.resetSettings();
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
  ESPDash.updateTemperatureCard("temp1", readDHTemperatureC());
  delay(3000);
}
