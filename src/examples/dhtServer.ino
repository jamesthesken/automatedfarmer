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

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// To avoid hardcoded WiFi credentials: https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h> 


AsyncWebServer server(80);
DNSServer dns;

float readDHTemperatureC() {
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


void setup() {
  Serial.begin(115200);

  // Initialize the sensor
  dht.begin();

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
