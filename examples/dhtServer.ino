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

#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#define rx 5                                          //define what pin rx is going to be
#define tx 4                                          //define what pin tx is going to be
SoftwareSerial myserial(rx, tx);                      //define how the soft serial port is going to work

// To avoid hardcoded WiFi credentials: https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h> 

AsyncWebServer server(80);
DNSServer dns;



// Get temperature event and print its value.
float readDHTemperatureC() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  float tempC = event.temperature;
  
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    float tempC = event.temperature;
  }

    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));  

  return float(tempC);
}

String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product
int Co2;                                              //used to hold a integer number that is the Co2

void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}

int co2Reading(){
  if (input_string_complete == true) {                //if a string from the PC has been received in its entirety
    myserial.print(inputstring);                      //send that string to the Atlas Scientific product
    myserial.print('\r');                             //add a <CR> to the end of the string
    inputstring = "";                                 //clear the string
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
  }

  if (myserial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }


  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
    Serial.println(sensorstring);                     //send that string to the PC's serial monitor
                                                    //uncomment this section to see how to convert the Co2 readings from a string to an  integer 
    if (isdigit(sensorstring[0])) {                   //if the first character in the string is a digit
      Co2 = sensorstring.toInt();                     //convert the string to a integer so it can be evaluated by the Arduino
      if (Co2 >= 400) {                               //if the Co2 is greater than or equal to 400
        Serial.println("high");                       //print "high" this is demonstrating that the Arduino is evaluating the Co2 as a number and not as a string
      }
      if (Co2 <= 399) {                               //if the Co2 is less than or equal to 399
        Serial.println("low");                        //print "low" this is demonstrating that the Arduino is evaluating the Co2 as a number and not as a string
      }
    }
    
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }

  return Co2;
}

void setup() {
  Serial.begin(9600);
  myserial.begin(9600);
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);

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
  ESPDash.addTemperatureCard("temp1", "Air Temperature", 0, 20);
  ESPDash.addNumberCard("num1", "CO2 (ppm)", 264);

  server.begin();
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {  
  MDNS.update(); 
    
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  ESPDash.updateNumberCard("num1", co2Reading());
  ESPDash.updateTemperatureCard("temp1", readDHTemperatureC());
  delay(3000);
}
