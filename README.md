# esp32 Automated Chicken coop

## General description

This Automated Chicken Coop consists of these features:
 + Automated coop door opening/closing depending on actual time.
 + Humidity and Temperature measuring inside the coop.

## Components used

+ esp32
+ ds1307 rtc module (ntp)
+ dht22 module (humidity and temperature)
+ 3 buttons (door, light, automated/manual controlling switch)
+ Car battery
+ 12v to 3.3V voltage converter
+ LCD 16x2
+ 2x relay module (light and doors)
+ 12V light
+ 12V motor for door opening/closing 

### Pin setup

Check ```include/variables.h``` file for pin settings.


