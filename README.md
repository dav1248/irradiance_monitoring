# Irradiance Monitoring System

Offline Irradiance Monitoring System working on Adafruit M0 Feather. The system is composed of:
* Adafruit M0 Feather
* Adafruit Adalogger RTC
* Adafruit OLED display
* 2x 4Wp mono-si module (to measure short-circuit current)
* 1x 1 Ohm (5W) shunt resistor (to measure short-circuit current)
* 1x lipo battery (to power the system)
* 1x 15Wp mono-si module (to power the system)
* 1x buck converter

![IMS](/images/ims.jpg)


## How to build

The principle of the monitoring system is very simple, and unlike the monitoring of the PV charging system, requires very few components to work properly.

![Schema](/images/schema_irr.png)

list of components
IMAGE OF CIRCUIT

## Code walkthrough

Before getting anything done, you can check the [Adafruit M0 page](https://learn.adafruit.com/adafruit-feather-m0-basic-proto) and follow the instructions on how to setup the M0.

The code for the monitoring system is quite simple and holds in one file (plus one config.h file). To run, you just have to click the Upload button on the Arduino IDE.

#### config.h

The *config.h* contains a few parameters that will influence the monitoring:

**sampling_interval**: defines the rate of sampling and logging to SD card. A 4s sampling rate offers good balance between accuracy and reasonable file length, especially when using Excel.

**isc_current**: the short-circuit current of the PV module

**RshuntPV1** and **RshuntPV2**: the value of the shunt resistors.

#### other parameters

#### setup()


#### loop()

For each loop, the program will check if the current time is inside the sleep window, if yes, it will go into low-power sleep mode. If not, it will enter the main logic:

* At each loop, check if *lastUpdate* was more than *sampling_interval* milliseconds ago. 
* If yes, read the ADC pins of the M0 and compute the potential difference at the resistance.
* From the previous values, compute the short circuit current and irradiance
* save the computed values into the *.tsv* file
* handles the OLED screen and buttons

By using *millis()* as our main time management tool, we can therefore achieve a seemingly uninterrupted program that updates in real time the values into the SD card and on the OLED display while listening for buttons inputs.



## Code Libraries

The following libraries are needed (just type the name in the Arduino IDE under tools->Manage Libraries)

* RTClib (RTC module)
* Adafruit SleepyDog
* Adafruit SSD1306 (OLED Display)
