#include <SPI.h>
#include <SD.h>
#include <Adafruit_SleepyDog.h>


/************************* System *********************************/
#define vbat A7
const int sampling_interval = 4000;
const double stc_current= 0.72; // Amps
const double stc_irradiance = 1000; // Watts per m2 
const double RshuntPV1=1; // Shunt Resistance for current meas. of PV module [Ohm]
const double RshuntPV2=1; // Shunt Resistance for current meas. of Loads [Ohm]


/************************* OLED Display ***************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
//#define BUTTON_A  9
#define BUTTON_B  6 
#define BUTTON_C  5
double display_awake = 0;

/************************* SD Card *********************************/
// Set the pins used
#define cardSelect 10 
File logfile;

/************************* RTC *************************************/
// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int offset_year=2000; // offset year for rtc formatting

const int wakeupHour = 6;
const int sleepHour = 18;
