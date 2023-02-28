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
#include <U8x8lib.h>
#include <PCF8563.h>
PCF8563 pcf;
#include <Wire.h>
#if defined(ARDUINO_SEEED_XIAO_M0) && SAMD_REG_DUMP
#include <checkregs.h>
#endif
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

#define LOOP_DELAY 1000        // ms to delay at end of loop
#define BUTTON_INTERRUPT 1     // 0 to use polling, 1 to user interrupts
#define BUTTON_INTERRUPT_MODE FALLING
#define BUTTON_PIN A1  // D1 not defined for XIAO_M0
#if defined(ARDUINO_XIAO_ESP32C3)
#define LED_PIN (-1)    // undefined
#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_SEEED_XIAO_RP2040)
#define LED_PIN (17)  //xiao pin    ... avoid conflict with PICO W 
#else
#define LED_PIN    LED_BUILTIN
#endif

#define BUZZER_PIN A3   //D3 didn't work for SAMD
#define DAC_PIN    A0   // only on SAMD
#define DAC_RESOLUTION 10  // only SAMD21
#define ADC_PIN    A2
// default ADC_RESOLUTION=12 for XIAO_M0
//                        12 for NRF mbed and NRF adafruit core
//                        12 for rp2040
//                        12 for ESP32C3 ... not change able
#if defined(ARDUINO_XIAO_ESP32C3)
#define MYADCRESOLUTION 12
#else
#define MYADCRESOLUTION ADC_RESOLUTION
#endif

#if defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_SEEED_XIAO_RP2040)
// use XIAO pinout
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ 7u, /* data=*/ 6u, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#elif defined(ARDUINO_XIAO_ESP32C3)
//0U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#else
// use default I2C
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#endif


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


static void dumpinfo(void) {
#if defined(ARDUINO_SEEED_XIAO_M0)
  Serial.println("XIAO_M0");
#elif defined(ARDUINO_XIAO_ESP32C3)
  Serial.println("XIAO_ESP32C3");
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  Serial.println("PICO");
#elif defined(ARDUINO_SEEED_XIAO_RP2040)
  Serial.println("XIAO_RP2040");
#elif defined(SEEED_XIAO_NRF52840_SENSE)
  Serial.println("XIAO_NRF52840_SENSE (mbed)");
#elif defined(ARDUINO_Seeed_XIAO_nRF52840_Sense)
  Serial.println("ARDUINO_Seeed_XIAO_nRF52840_Sense adafruit core");
#else
  Serial.println("Unknown board");
#endif

  Serial.print("BUTTON_INTERRUPT = ");Serial.print(BUTTON_INTERRUPT);
  Serial.print("   Count=");Serial.println(buttonCt);
  Serial.print("Button Pin: ");Serial.print(BUTTON_PIN);
  Serial.print("     digitalInterrupt: ");
  Serial.print(digitalPinToInterrupt(BUTTON_PIN));
  Serial.print("  Value=");Serial.println(digitalRead(BUTTON_PIN));
  Serial.print("FALLING=");Serial.println(FALLING);
  Serial.print("LED Pin: ");Serial.println(LED_PIN);
  Serial.print("Buzzer Pin: ");Serial.println(BUZZER_PIN);
#if defined (ARDUINO_SEEED_XIAO_M0)
  Serial.print("DAC Pin: ");Serial.print(DAC_PIN);
  Serial.print("  Resolution: ");Serial.println(DAC_RESOLUTION);
#else
  Serial.println("No DAC");
#endif
  Serial.print("ADC Pin: ");Serial.println(ADC_PIN);
  Serial.print("  Resolution: ");Serial.println(MYADCRESOLUTION);
}

void setup() {
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
#if not defined(ARDUINO_XIAO_ESP32C3)
  pinMode(LED_PIN, OUTPUT);
#endif
#if defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_SEEED_XIAO_RP2040)
  pinMode(25, OUTPUT);digitalWrite(25, 1);  // turn off all
  pinMode(16, OUTPUT);digitalWrite(16, 1);
  digitalWrite(LED_PIN, 1);
#endif
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ADC_PIN, INPUT);  // do not set mode of DAC output  
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin();
  dumpinfo();

#if BUTTON_INTERRUPT
#if defined(ARDUINO_XIAO_ESP32C3)
  attachInterrupt(BUTTON_PIN, buttonpush, FALLING);
#else
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonpush, FALLING);
#endif
#endif
#if not defined(ARDUINO_XIAO_ESP32C3)
  analogReadResolution(MYADCRESOLUTION);   // should be 12
#if defined(ARDUINO_SEEED_XIAO_M0)
  analogWriteResolution(DAC_RESOLUTION);  // should be 10
#endif
#endif
  u8x8.begin();
  Serial.println("u8x8 begin ... done");
  u8x8.setFlipMode(1);
  pcf.init();//initialize the clock
  Serial.println("pcf.init ... done");
  pcf.stopClock();//stop the clock
  pcf.setYear(22);//set year
  pcf.setMonth(2);//set month
  pcf.setDay(14);//set dat
  pcf.setHour(10);//set hour
  pcf.setMinut(11);//set minut, note missing e!
  pcf.setSecond(12);//set second
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
 
void loop() {
  int newVoltage = -1;
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
  u8x8.setFont(u8x8_font_chroma48medium8_r);   // choose a suitable font
 
  u8x8.setCursor(0, 0);
  u8x8.print(nowTime.day);
  u8x8.print("/");
  u8x8.print(nowTime.month);
  u8x8.print("/");
  u8x8.print("20");
  u8x8.print(nowTime.year);
  u8x8.setCursor(0, 1);
  u8x8.print(nowTime.hour);
  u8x8.print(":");
  u8x8.print(nowTime.minute);
  u8x8.print(":");
  u8x8.print(nowTime.second);
  
  u8x8.setCursor(0,2);
  // digitalWrite(LED_PIN, buttonState);
  // u8x8.print("BTN: ");u8x8.print(buttonState);

  if (newVoltage >= 0) {
    u8x8.setCursor(0,3);
    u8x8.clearLine(3);
#if defined(ARDUINO_SEEED_XIAO_M0)
    analogWrite(DAC_PIN, newVoltage);
#else
    Serial.println("No DAC on this chip");
#endif
    u8x8.print("vDAC: ");u8x8.print(newVoltage);
  }
  int adclevel = analogRead(ADC_PIN);
  u8x8.clearLine(4);
  u8x8.setCursor(0,4);
  u8x8.print("  ANLG: ");u8x8.print(adclevel);
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
  delay(LOOP_DELAY);
}
