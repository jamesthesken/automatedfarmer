# Some Examples
These pieces of code represent some examples for using ESP-DASH, LCD screens, DHT-11's, and other sorts.

## DataLogger
`datalogger.ino` contains a mix of two great libraries:
1. [ESP-DASH](https://github.com/ayushsharma82/ESP-DASH): An easy way to create web-based dashboards for the ESP8266/32.
2. [SPIFFSLogger](https://github.com/bitmario/SPIFFSLogger): SPIFFS storage made simple on the ESP8266.

We leverage the SPIFFSLogger library to store DHT11 sensor data every 5 minutes. The ESP-DASH library gives us both real-time value display and a line chart showing the sensor data over 30 minutes. 

### What could be improved/where to investigate
While it's convenient to store sensor data on the device itself, is it considered bad practice? For a resource constrained system, perhaps not. However I'm just the guy that throws Arduino libraries together. I do not know yet.

If you want a local dashboard that has datalogging capabilities this might be the solution for you.

## AutFarm
`autoFarm.ino` contains code to drive an 16x2 LCD for the Arduino. Nothing special really, but it's something I've looked up a lot for i2c connections. 
