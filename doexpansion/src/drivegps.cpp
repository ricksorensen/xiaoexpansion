// https://wiki.seeedstudio.com/Seeeduino-XIAO-Expansion-Board/
// needs u8g2, pcf8563 libraries
// Arduino used https://github.com/Bill2462/PCF8563-Arduino-Library
// rename to .ino for Arduino IDE

/** Platform IO # defines:
    ARDUINO_SEEED_XIAO_M0              default seeed_xiao
    ARDUINO_XIAO_ESP32C3               default seeed_xiao_esp32c3
    ARDUINO_RASPBERRY_PI_PICO          default pico build (official pi, mbed)
    ARDUINO_SEEED_XIAO_RP2040          rp2040 earlephilhower tools
    ARDUINO_Seeed_XIAO_nRF52840_Sense  arduino IDE, platformio adafruitnrf
    SEEED_XIAO_NRF52840_SENSE          platformio, mbed nrf
**/
#if !defined(SAMD_REG_DUMP)
#define SAMD_REG_DUMP 0
#endif
#include <Arduino.h>
#include "playsong.h"
#include <TinyGPSPlus.h>
#include <PCF8563.h>
PCF8563 pcf;


#include "xiaosamd.h"
#include "xiaorp2040.h"
#include "xiaorp2350.h"
#include "xiaoesp32c3.h"
#include "xiaoesp32s3.h"
#include "xiaonrf52.h"

OLED oled(128,64);

/** Interrupt states:
    M0, RP2040, NRF52840
    // LOW = 0x00 for LOW level
    // HIGH = 0x01 for HIGH level
    // CHANGE = 0x02
    // RISING = 0x04
    // FALLING = 0x03
    ESP32C3 interrupt states:
    // DISABLED = 0x00 (== LOW)
    // ONLOW = 0x04 for LOW level
    // ONHIGH = 0x05 for HIGH level
    // CHANGE = 0x03
    // RISING = 0x01
    // FALLING = 0x02
**/
#if not defined(GPSSerial)
#define GPSSerial Serial1
#endif
TinyGPSPlus GPS;

#define LOOP_DELAY 1000        // ms to delay at end of loop
#define BUTTON_INTERRUPT 1     // 0 to use polling, 1 to user interrupts
#define BUTTON_INTERRUPT_MODE FALLING


static bool buzzOn = false;
static volatile unsigned long buttonCt=0;

#if BUTTON_INTERRUPT
static volatile bool buttonState = false;
#if defined(ARDUINO_XIAO_ESP32C3)
static void IRAM_ATTR buttonpush(void) {
  buttonState = true;
  buttonCt++;
}
#else
static void buttonpush(void) {
  buttonState = true;
  buttonCt++;
}
#endif
#endif
//static char gpshour, gpsminute, gpssecond;

static inline void clearSerial(void) {
  while (GPSSerial.available() > 0) {
    (void) GPSSerial.read();
  }
  while (GPSSerial.available() == 0) {
  }
}

static void getgps(unsigned long mswait) {
  //clearSerial();
  //Serial.print("Serial Avail: ");Serial.println(GPSSerial.available());
  unsigned long start = millis();
  do {
    while (GPSSerial.available() > 0) {
      char c=GPSSerial.read();
#if defined(GPSDEBUG)      
      Serial.write(c);
#endif
      GPS.encode(c);
    }
  } while (millis() - start < mswait);
#if defined(GPSDEBUG)
  Serial.println();
  Serial.print("  GPS Char Processed: ");Serial.print(GPS.charsProcessed());
  Serial.print("sentwithFix: ");Serial.println(GPS.sentencesWithFix());
#endif
}


//char tstr[8];
static void dumpinfo(void) {

  //tstr[0] = 0;
  // assume CST ... %24 for 0-23 hours
  //while (GPSSerial.available() > 0) {
  //  GPSSerial.read();
  //}
  //while (GPSSerial.available() == 0) {
  //}
  //snprintf(tstr,8,"%02d:%02d",gpshour, gpsminute);
  //Serial.print("save gps Time ");
  //Serial.println(tstr);
  Serial.print(F("GPS  "));
  if (GPS.location.isValid()) {
    Serial.println(" Position Valid");
  } else {
    Serial.println(" Position not Valid");
  }    
  if (GPS.date.isValid()) {
    Serial.print(" Date Valid: ");
    Serial.print(GPS.date.month());
    Serial.print(F("/"));
    Serial.print(GPS.date.day());
    Serial.print(F("/"));
    Serial.print(GPS.date.year());
  } else {
    Serial.print(" No valid GPS Date");
  }
  Serial.print(" ");
  if (GPS.time.isValid()) {
    if (GPS.time.hour() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.hour());
    Serial.print(F(":"));
    if (GPS.time.minute() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.minute());
    Serial.print(F(":"));
    if (GPS.time.second() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.second());
  }
  Serial.println();

  Serial.print("BUTTON_INTERRUPT = ");Serial.print(BUTTON_INTERRUPT);
  Serial.print("   Count=");Serial.println(buttonCt);
  Serial.print("Button Pin: ");Serial.print(BUTTON_PIN);
  Serial.print("     digitalInterrupt: ");
  Serial.print(digitalPinToInterrupt(BUTTON_PIN));
  Serial.print("  Value=");Serial.println(digitalRead(BUTTON_PIN));
  Serial.print("FALLING=");Serial.println(FALLING);
  Serial.print("LED Pin: ");Serial.println(LED_PIN);
  Serial.print("Buzzer Pin: ");Serial.println(BUZZER_PIN);
  Serial.print("ADC Pin: ");Serial.println(ADC_PIN);
  Serial.print("  Resolution: ");Serial.println(MYADCRESOLUTION);
}

void setup() {
  // GPSSerial.begin(9600);
  Serial.begin(115200);
  while (!Serial) ;        // ESP32C3 Serial seems up if powered from USB with no terminal pgm connected
  delay(5000);
  Serial.println("setup: Starting up");
#if defined(ARDUINO_SEEED_XIAO_M0) && SAMD_REG_DUMP
  portstat();
  for (int i=0;i<12;i++) {
    portregs(i);
  }
  dacstate();
#endif
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ADC_PIN, INPUT);  // do not set mode of DAC output  
  pinMode(BUZZER_PIN, OUTPUT);

  //Wire.begin();
  dumpinfo();

#if BUTTON_INTERRUPT
#if defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32S3)
  attachInterrupt(BUTTON_PIN, buttonpush, FALLING);
#else
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonpush, FALLING);
#endif
#endif
  TwoWire* myI2C=mcusetup();
  Serial.print("pcf.init ...");
  pcf.init(myI2C);
  Time pcfTime = pcf.getTime();//get current time
  Serial.println("....done \n Starting OLED");
  oled.setWire(myI2C);
  oled.begin();
  Serial.println("Clearing OLED");
  oled.clearScr();
  // year is a byte- if month bit 7 is set, year 19xx, else (default) is 20xx
  if (pcfTime.year < 20) {
    pcf.stopClock();//stop the clock
    pcf.setYear(22);//set year
    pcf.setMonth(2);//set month
    pcf.setDay(14);//set dat
    pcf.setHour(10);//set hour
    pcf.setMinut(11);//set minut, note missing e!
    pcf.setSecond(12);//set second
  }
  pcf.startClock();//start the clock
  Serial.println("pcf.startClock() ... done");
#if defined(ARDUINO_SEEED_XIAO_M0) && SAMD_REG_DUMP
  portstat();
  for (int i=0;i<12;i++) {
    portregs(i);
  }
  dacstate();
#endif  
  Serial.println("Enter integer voltage values");
}

char tstr[20];
void loop() {
  int newVoltage = -1;
  getgps(LOOP_DELAY);
  if (Serial.available()>0) {
    newVoltage=Serial.parseInt();
    // flush input
    while (Serial.available() > 0) {
      Serial.read();
    } 
    if (newVoltage > 1023) {
      newVoltage = 1023;
    }
    Serial.print("New Voltage:");
    Serial.println(newVoltage);
    dumpinfo();
#if defined(ARDUINO_SEEED_XIAO_M0) && SAMD_REG_DUMP
    dacstate();
#endif  
    
  }
  Time nowTime = pcf.getTime();//get current time
  sprintf(tstr,"rtc %02d/%02d/20%02d",nowTime.day,nowTime.month,nowTime.year); 
  oled.print(tstr,0,6);
  sprintf(tstr,"   %02d:%02d:%02d",nowTime.hour,nowTime.minute,nowTime.second);
  oled.print(tstr,0,16);
  if (GPS.date.isValid()) {
    sprintf(tstr,"gps %02d/%02d/%04d",GPS.date.day(),GPS.date.month(),GPS.date.year());
    oled.print(tstr,0,26);
  }
  if (GPS.time.isValid()) {
    sprintf(tstr,"   %02d:%02d:%02d",GPS.time.hour(),GPS.time.minute(),GPS.time.second());
    oled.print(tstr,0,36);
  }

  if (newVoltage >= 0) {
#if defined(ARDUINO_SEEED_XIAO_M0)
    analogWrite(DAC_PIN, newVoltage);
#else
    Serial.println("No DAC on this chip");
#endif
    sprintf(tstr, "vDAC: %04d",newVoltage);
    oled.print(tstr,0,46);
  }
  sprintf(tstr,"  ANLG: %04u",analogRead(ADC_PIN));
  oled.print(tstr,0,56);
  oled.inflate();
#if BUTTON_INTERRUPT
  if (buttonState) {
    buttonState = false;
    buzzOn = !buzzOn;
  }
#else
  if (digitalRead(BUTTON_PIN) == LOW) {
    buzzOn = !buzzOn;
  }
#endif
#if not defined(ARDUINO_XIAO_ESP32C3)
  digitalWrite(LED_PIN, digitalRead(BUTTON_PIN));
#endif
  if (buzzOn) {
    doBuzzer(BUZZER_PIN);
  }
  // delay(LOOP_DELAY);
}
