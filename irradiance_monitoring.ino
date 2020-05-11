/*
 * Code for an irradiance measuring device reading analog inputs of 
 * two PV modules plugged in parallel with a resistor on pins (A0,A2) and (A1,A3). 
 */
 
#include "config.h"


/************************* System Parameters *********************************/
double lipo_voltage = 0;
unsigned long lastUpdate = 0;
unsigned long day0;
int sleepMS=0;

/************************* Measurement Parameters ****************************/
int analogValueA0 = 0;
int analogValueA1 = 0;
int analogValueA2 = 0;
int analogValueA3 = 0;


int voltageValue1 = 0;
int voltageValue2 = 0;

double PVvoltage1 = 0;
double PVvoltage2 = 0;

double PVcurrent1 = 0;
double PVcurrent2 = 0;

double irradiance1 = 0;
double irradiance2 = 0;



/**********************************************************************/
/************************* Setup **************************************/
/**********************************************************************/
void setup() {

  Serial.begin(9600);
  delay(3000);
  Serial.println("\r\nAnalog logger test");
  pinMode(13, OUTPUT);

  analogReadResolution(12); // default analog read resolution is 10bits, but M0 supports up to 12

  // OLED INIT // 
  Serial.println("OLED connection");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println("OLED begun");

  display.clearDisplay();   // Clear the buffer.
  display.display();

  //pinMode(BUTTON_A, INPUT_PULLUP); pin of Button A already used
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display init
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("hiLyte irradiance");
  display.println("logger");

  // SD CARD INIT // 
  if (!SD.begin(cardSelect)) {   // see if the card is present and can be initialized:
    Serial.println("No card inserted!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("No card inserted!");
    display.display();
    delay(3000);
    error(2);
  }
  char filename[15];
  strcpy(filename, "/irr000.tsv");   // creates file
  for (uint8_t i = 0; i < 1000; i++) {
    filename[4] = '0' + i / 100;
    filename[5] = '0' + i / 10;
    filename[6] = '0' + i % 10;
    if (! SD.exists(filename)) {     // create if does not exist, do not open existing, write, sync after write
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if ( ! logfile ) {
    Serial.print("Couldnt create ");
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to ");
  Serial.println(filename);

  init_logfile();   // inits the log file with data headers

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.println("Ready!");


  // display the name of the newly created file
  display.println("file :");
  display.println(filename);
  display.display();
  delay(2000);

  // RTC INIT //
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //can use this line to recalibrate date&time
    //rtc.adjust(DateTime(2020, 1, 1, 23, 59, 50));    //to do so, remove the small battery 
  }
  day0 = rtc.now().unixtime()/(24*60*60); // rtc snapshot of current day
}


/**********************************************************************/
/************************* Loop ***************************************/
/**********************************************************************/
void loop() {

  // LOW-POWER MODE //
  // the device is programmed to sleep during the night while waking up automatically
  int current_hour = rtc.now().hour();
  if (!(current_hour>=wakeupHour && current_hour<=sleepHour)){    // looks if has to wake up 
    check_reset(day0);  // checks if has to reset the measuring system to start a new day
    Serial.println("going back to bed");
    sleepDisplay(&display);
    int sleepMS = Watchdog.sleep(); // goes back to bed
  } else{                                                         // wakes up
    
    wakeDisplay(&display);     //wake display
    display.display();
    
    
    // MEASUREMENTS //
    if (millis() > (lastUpdate + sampling_interval)) {
  
  
      lipo_voltage = (3.3 / 4096.0) * (2 * analogRead(vbat)); // calculates lithium battery voltage
  
      analogValueA0 = analogRead(0); // connected to PV1+
      analogValueA1 = analogRead(1); //   connected to PV2+
      analogValueA2 = analogRead(2); // connected to PV1-
      analogValueA3 = analogRead(3); //   connected to PV2-
  
      voltageValue1 = analogValueA0 - analogValueA2; // voltage value of PV1 in counts
      voltageValue2 = analogValueA1 - analogValueA3; // voltage value of PV2 in counts
  
      PVvoltage1 = 3.3 * (voltageValue1) / 4096; // voltage of PV1 in [V]
      PVcurrent1 = PVvoltage1 / RshuntPV1;       // current of PV1 in [A]
  
      PVvoltage2 = 3.3 * (voltageValue2) / 4096; // voltage of PV1 in [V]
      PVcurrent2 = PVvoltage2 / RshuntPV2;       // current of PV1 in [A]
  
      irradiance1 = irradiance(PVcurrent1);      // irradiance of PV1 in [W]
      irradiance2 = irradiance(PVcurrent2);      // irradiance of PV2 in [W]
  
      input_logfile(); // saves the values into file in SD card
  
      lastUpdate = millis(); // change last update value
    }

    UI_management(); // management of button input and text display
  }
}


/**********************************************************************/
/************************* Helper Functions ***************************/
/**********************************************************************/

// inits the logfile with correct columns
void init_logfile() {
  logfile.print("date \t");
  logfile.print("time \t");
  logfile.print("pv1_W \t");
  logfile.print("pv2_W \t");
  logfile.print("pv1_V \t");
  logfile.print("pv1_C \t");
  logfile.print("pv2_V \t");
  logfile.print("pv2_C \t");
  logfile.print("vbat \t");
  logfile.println();
}

// saves the values into file in SD card with a timestamp
void input_logfile(){

      DateTime now = rtc.now(); // rtc snapshot
  
      // rtc
      logfile.print(now.day());
      logfile.print(".");
      logfile.print(now.month());
      logfile.print(".");
      logfile.print((now.year() - offset_year), DEC);
      logfile.print("\t");
      logfile.print(now.hour(), DEC);
      logfile.print(":");
      logfile.print(now.minute(), DEC);
      logfile.print(":");
      logfile.print(now.second(), DEC);
      logfile.print("\t");
  
      // values
      logfile.print(irradiance1, 2);
      logfile.print("\t");
      logfile.print(irradiance2, 2);
      logfile.print("\t");
      logfile.print(PVvoltage1, 3);
      logfile.print("\t");
      logfile.print(PVcurrent1, 3);
      logfile.print("\t");
      logfile.print(PVvoltage2, 3);
      logfile.print("\t");
      logfile.print(PVcurrent2, 3);
      logfile.print("\t");
      logfile.print(lipo_voltage);
      logfile.println("");
      logfile.flush();
}

// displays information on OLED display and handles button inputs
void UI_management(){
  
    //info: Button A of the OLED is connected to a pin already used by the program 
      
    if (!digitalRead(BUTTON_B)) //first button
    {
      DateTime current_time= rtc.now();
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(current_time.hour(), DEC);
      display.print(":");
      display.print(current_time.minute(), DEC);
      display.print(":");
      display.println(current_time.second(), DEC);
      display.print(current_time.day());
      display.print(".");
      display.print(current_time.month());
      display.print(".");
      display.print((current_time.year() - offset_year), DEC);
      
    }
    else if (!digitalRead(BUTTON_C)) //second button
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("vbat= "); display.print(lipo_voltage); display.println(" [V]");
    } else {                         // persistant display

      display.clearDisplay();      
      display.setCursor(0, 0);
      display.print("P1: ");
      display.print(irradiance1);
      display.println(" [W/m2]");
      display.print("P2: ");
      display.print(irradiance2);
      display.println(" [W/m2]");
  
      display.print("vbat: "); display.print(lipo_voltage); display.println(" [V]");
      
      display.print("isc: "); display.print(PVcurrent1,3); display.println(" [A]");
    }
  
    delay(10);
    yield();
    display.display();
}

// returns the irradiance of the current sample in W/m2
double irradiance(double current) {
  return current * stc_irradiance / stc_current;
}

// returns the energy of the current interval in Wh
double energy(double power, int sampling_interval, double energy_tot) {
  return energy_tot + power * (sampling_interval / 1000) / 3600;
}


// checks if the device has to reset and create a new file
// called only when sleeping, and triggered only once per night when passing from 23h59 to 00h00 of the next day
void check_reset(unsigned long day0){

  unsigned long current_time = rtc.now().unixtime();
  unsigned long current_day = current_time/(24*60*60);
  
  if(current_day-day0>=1){
    int countdownMS = Watchdog.enable(1000);
    wakeDisplay(&display);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("resetting");
    display.display();
    delay(500);
    sleepDisplay(&display);
    delay(2000);
  }
  
}

// error function
void error(uint8_t errno) {

  uint8_t i;
  for (i = 0; i < errno; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
  for (i = errno; i < 10; i++) {
    delay(200);
  }
}

// display sleep and wake functions
void sleepDisplay(Adafruit_SSD1306* display) {
  display_awake=0;
  display->ssd1306_command(SSD1306_DISPLAYOFF);
}
void wakeDisplay(Adafruit_SSD1306* display) {
  if (display_awake==0){
    display->ssd1306_command(SSD1306_DISPLAYON);
    display_awake=1;
  }
}
