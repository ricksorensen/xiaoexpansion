// https://wiki.seeedstudio.com/Seeeduino-XIAO-Expansion-Board/
// needs u8g2, pcf8563 libraries
// Arduino used https://github.com/Bill2462/PCF8563-Arduino-Library
// rename to .ino for Arduino IDE

#define LOOP_DELAY 1000   // ms to delay at end of loop
/** Platform IO # defines:
    ARDUINO_SEEED_XIAO_M0      default seeed_xiao
    ARDUINO_XIAO_ESP32C3       default seeed_xiao_esp32c3
    ARDUINO_RASPBERRY_PI_PICO  default pico build (official pi)
    ARDUINO_SEEED_XIAO_RP2040  rp2040 earlephilhower tools
**/
#include <Arduino.h>
#include <U8x8lib.h>
#include <PCF8563.h>
PCF8563 pcf;
#include <Wire.h>
/** Interrupt states:
    M0, RP2040:
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

#define BUTTON_INTERRUPT 1
#define BUTTON_INTERRUPT_MODE FALLING
#define BUTTON_PIN A1   // D1 not defined for XIAO_M0
#if defined(ARDUINO_XIAO_ESP32C3)
#define LED_PIN (-1)    // undefined
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
#define LED_PIN (17)  //xiao pin
#else
#define LED_PIN    LED_BUILTIN
#endif

#define BUZZER_PIN A3   //D3 didn't work
#define DAC_PIN    A0
#define DAC_RESOLUTION 10  // only SAMD21
#define ADC_PIN    A2

#if defined(ARDUINO_RASPBERRY_PI_PICO)
// use XIAO pinout
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ 7u, /* data=*/ 6u, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#else
// use default I2C
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#endif

int length = 28; // the number of notes
char notes[] = "GGAGcB GGAGdc GGxecBA yyecdc";
int beats[] = { 2, 2, 8, 8, 8, 16, 1, 2, 2, 8, 8, 8, 16, 1, 2, 2, 8, 8, 8, 8, 16, 1, 2, 2, 8, 8, 8, 16 };
int tempo = 150;
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(tone);
  }
}
 
void playNote(char note, int duration) {
  char names[] = {'C', 'D', 'E', 'F', 'G', 'A', 'B',
                  'c', 'd', 'e', 'f', 'g', 'a', 'b',
                  'x', 'y'
  };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014,
                  956,  834,  765,  593,  468,  346,  224,
                  655 , 715
  };
  int SPEE = 5;
 
  // play the tone corresponding to the note name
 
  for (int i = 0; i < 16; i++) {
    if (names[i] == note) {
      int newduration = duration / SPEE;
      playTone(tones[i], newduration);
    }
  }
}

static bool buzzOn = false;
void doBuzzer(void) {
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }
    // pause between notes
    delay(tempo);
  }
}

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
  
}

void setup() {
  Serial.begin(115200);
  while (!Serial) ;        // ESP32C3 Serial seems up if powered from USB with no terminal pgm connected

#if not defined(ARDUINO_XIAO_ESP32C3)
  pinMode(LED_PIN, OUTPUT);
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
  analogReadResolution(ADC_RESOLUTION);   // should be 12
#if not defined(ARDUINO_SEEED_XIAO_RP2040)
  analogWriteResolution(DAC_RESOLUTION);  // should be 10
#endif
#endif
  u8x8.begin();
  Serial.println("u8x8 begin ... done");
  u8x8.setFlipMode(1);
  pcf.init();//initialize the clock
  Serial.println("pcf.init ... done");
  pcf.stopClock();//stop the clock
  pcf.setYear(20);//set year
  pcf.setMonth(10);//set month
  pcf.setDay(23);//set dat
  pcf.setHour(17);//set hour
  pcf.setMinut(33);//set minut
  pcf.setSecond(0);//set second
  pcf.startClock();//start the clock
  Serial.println("pcf.startClock() ... done");
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
#if not defined(ARDUINO_SEEED_XIAO_RP2040)
    analogWrite(DAC_PIN, newVoltage);
#else
    Serial.println("No DAC on RP2040");
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
    doBuzzer();
  }
  delay(LOOP_DELAY);
}
