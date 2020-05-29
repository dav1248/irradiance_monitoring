# Irradiance Monitoring System

Offline Irradiance Monitoring System working on Adafruit M0 Feather. The system is composed of:
* Adafruit M0 Feather
* Adafruit Adalogger RTC
* Adafruit OLED display
* 2x 4Wp mono-si module (to measure short-circuit current)
* 2x 1 Ohm (5W) shunt resistor (to measure short-circuit current)
* 1x lipo battery (to power the system)
* 1x 15Wp mono-si module (to power the system)
* 1x buck converter


<img src="/images/ims.jpg" width="300" height="450">
<!---
![IMS](/images/ims.jpg)
-->

The system is able to log daily irradiance values as time series, while going to deep sleep at night. Without a PV power supply, the system should last around 4 to 5 days with a 2000mAh battery.



## How to build

The principle of the monitoring system is very simple, and unlike the monitoring of the PV charging system, requires very few components to work properly.

![Schema](/images/schema_irr.png)

The system uses the two 4W PV modules as the source of measurement (thus allowing for redundancy in case of data incoherence on one of the modules). It measures the current created by the module and going through the 1 Ohm resistor. This current is very close to the short-circuit current and hence if we know the short circuit current at STC (1000W/m2, 25°C), we can deduce the irradiance at any time since Isc is proportional to the irradiance present at the module surface.

1. Each module is wired in parallel with a 1 Ohm, 5W resistor. The positive side of the module is connected to A0, and the negative side  to A1 for the first module, and (A2, A3) for the second one respectively.
2. The negative side of each module is also wired to the uC GND, to get a common ground and avoid floating voltage.
3. The 15W power supply module is plugged into the buck converter, itself connected via USD to the uC.
4. The battery is plugged into the corresponding JST port of the uC.

![IMS](/images/irradiance_monitoring_circuit.png)
*Above is an illustrative diagram of the wiring (only relevant parts have been shown)*

I put everything into a waterproof plastic case, and built a support for the modules as you can see in the first picture.


## Code walkthrough

Before getting anything done, you can check the [Adafruit M0 page](https://learn.adafruit.com/adafruit-feather-m0-basic-proto) and follow the instructions on how to setup the M0.

The code for the monitoring system is quite simple and holds in one file (plus one config.h file). To run, you just have to click the Upload button on the Arduino IDE.

#### config.h

The *config.h* contains a few parameters that will influence the monitoring:

**sampling_interval**: defines the rate of sampling and logging to SD card. A 4s sampling rate offers good balance between accuracy and reasonable file length, especially when using Excel.

**isc_current**: the short-circuit current of the PV module. Should verify this value with a multimeter.

**RshuntPV1** and **RshuntPV2**: the value of the shunt resistors.

**wakeupHour** and **sleepHour**: hours that define wake up and sleeping time of the monitoring system.



#### setup()

SD card reader: the code looks at existing log files and creates a new one with an incremented number, before opening it to insert the headers.

#### loop()

For each loop, the program will check if the current time is inside the sleep window, if yes, it will go into low-power sleep mode. If not, it will enter the main logic:

* At each loop, check if *lastUpdate* was more than *sampling_interval* milliseconds ago. 
* If yes, read the ADC pins of the M0 and compute the potential difference at the resistance.
* From the previous values, compute the short circuit current and irradiance
* save the computed values into the *.tsv* file
* handles the OLED screen and buttons

By using *millis()*, we get a seemingly uninterrupted program that updates in real time the values into the SD card and on the OLED display while listening for buttons inputs.


## Code Libraries

The following libraries are needed (just type the name in the Arduino IDE under tools->Manage Libraries)

* RTClib (RTC module)
* Adafruit SleepyDog
* Adafruit SSD1306 (OLED Display)


## Output file

a daily .TSV file (tab-separated file, ex: IRR001.tsv) with each rown containing date, time, irradiance from module 1 and 2, battery SoC, and various other parameters. You can use the Excel template to get a good quick look of the obtained data, just copy-paste the relevant columns into the template. The system creates a new TSV file every day at midnight.

The obtained power logged as P1 and P2 is the irradiance in [W/m2] computed based on the short circuit current **Isc** as a linear relationship between module current and sun irradiance. **It does not account for any losses or other effect**. Thus, if Isc varies from the rating of the module at STC (1000W/m2, 25°C), You have to make sure to apply a coefficient to the obtained data.


## Note on results

It is good to take into account some collection losses due to shadowing at low sun angles and MP voltage drop due to temperature. I tried to calibrate mine and compare with satellite irradiance values of [Solcast](https://www.solcast.com).
I am taking 7% losses due to shading and 3% du to temperature losses, for a total of 10%.