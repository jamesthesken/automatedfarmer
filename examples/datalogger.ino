#include <FS.h>
#include <SPIFFSLogger.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include "DHT.h"

#define DHTPIN 2        // NodeMCU Pin D4
#define DHTTYPE DHT11   // DHT 11

// Insert your wifi credentials
#define SSID ""
#define PSK ""

#define LOGGING_INTERVAL 30000

AsyncWebServer server(80);

// struct that defines the data we would like to log
struct MyData
{
    int temp;
    int humidity;
};

// create a new logger which will store records of our MyData type in files with names like
// /log/mydata/YYYYMMDD, keeping 1 day of history
SPIFFSLogger<MyData> logger("/log/sensordata", 1);

unsigned long lastLog = 0;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Line Chart Data
int x_axis_size = 7;
String x_axis[7] = {"35min", "30min", "25min", "20min", "15min", "10min", "5min"}; 
int y_axis_size = 7;
int y_axis[7] = {27, 25, 20, 22, 28, 28, 25};

void wifiSetup()
{
    WiFi.begin(SSID, PSK);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
}

void timeSetup()
{
    // details on ESP8266 time: https://github.com/d-a-v/Arduino/blob/goodies/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino

    // automatically sync UTC time using NTP
    configTime(0, 0, "pool.ntp.org");

    // optionally, we could set a timezone as well, e.g.:
    // setenv("TZ", "WET0CET-1,M3.5.0/2:00:00,M10.5.0/2:00:00", 1);
    // tzset();
}

void setup()
{
    Serial.begin(115200);

    // we need wifi+NTP (or local/RTC) ESP8266 time
    wifiSetup();
    timeSetup();

    // we must always initialize the SPIFFS before doing anything with the logger
    SPIFFS.begin();

    // initialize our logger
    logger.init();

    // our data record size
    Serial.printf("sizeof(MyData): %u\n", sizeof(MyData));
    // time_t timestamp + our data, as stored in SPIFFS
    Serial.printf("sizeof(SPIFFSLogData<MyData>): %u\n", sizeof(SPIFFSLogData<MyData>));

    dht.begin();

    ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance
    // Add Respective Cards
    ESPDash.addLineChart("chart1", "Air Temperature", x_axis, x_axis_size, "Air Temp. °C", y_axis, y_axis_size);
    server.begin();
}

void loop()
{
    const unsigned long currentMillis = millis();

    if (currentMillis - lastLog > LOGGING_INTERVAL)
    {
        // Reading temperature or humidity takes about 250 milliseconds!
        int h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        int t = dht.readTemperature();

        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t)) {
          Serial.println(F("Failed to read from DHT sensor!"));
         return;
        }

        // store the values if they are valid
        struct MyData data = {
            t, // temp
            h  // humidity
        };
        // log the data
        Serial.printf("Writing temp: %.1f, humidity: %.1f\n", data.temp, data.humidity);
        logger.write(data);

        // fetch and display last values
        const time_t now = time(nullptr);
        const size_t rowCount = logger.rowCount(now);
        struct SPIFFSLogData<MyData> readback;

        // read 1 item starting at index rowCount-1 from the logfile corresponding to the date now
        // and into the readback variable
        logger.readRows(&readback, now, rowCount - 1, 1);
        Serial.printf("Readback time: %d, temp: %.1f, humidity: %.1f\n",
                      readback.timestampUTC,
                      readback.data.temp,
                      readback.data.humidity);

        Serial.printf("Current logfile has %u rows.\n", rowCount);

        // filtering example
        logger.readRowsBetween(&readback,      // output
                               now - (60 * 5), // time start (inclusive)
                               now,            // time end (inclusive)
                               0,              // start index
                               1               // max number of rows to fetch (size your output buffer accordingly!)
        );
        Serial.printf("First record in the last 5 minutes: %d, temp: %d, humidity: %d\n",
                      readback.timestampUTC,
                      readback.data.temp,
                      readback.data.humidity);

        Serial.println();
        
        for (int i=0; i < 7; i++) {
          logger.readRowsBetween(&readback,      // output
                               now - (60 * i), // time start (inclusive)
                               now,            // time end (inclusive)
                               0,              // start index
                               1               // max number of rows to fetch (size your output buffer accordingly!)
          );
          Serial.printf("TS: %d, T: %d, H: %d\n",
                        readback.timestampUTC,
                        readback.data.temp,
                        readback.data.humidity);
         // Fill Data 
         y_axis[i] = readback.data.temp;
        }

        ESPDash.updateLineChart("chart1", x_axis, x_axis_size, y_axis, y_axis_size);

        lastLog = currentMillis;
      }

    // Processes file name changes and old file deletion according to the interval set
    // at initialization. Should always be called in the loop.
    logger.process();
}
