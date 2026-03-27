#if defined(ARDUINO_SEEED_XIAO_M0)

#include <Arduino.h>
#include <SSD1306.h>

#if SAMD_REG_DUMP
#include <checkregs.h>
#endif

#define GPSSerial Serial1

#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN LED_BUILTIN
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN A0              // only on SAMD
#define DAC_RESOLUTION 10          // only SAMD21
#define ADC_PIN A2
#define MYADCRESOLUTION ADC_RESOLUTION

inline static TwoWire* mcusetup(void) {
  Wire.begin();  
  Wire.setClock(400000);
  GPSSerial.begin(9600);
  pinMode(LED_PIN,OUTPUT);
  analogWriteResolution(DAC_RESOLUTION);
  analogReadResolution(MYADCRESOLUTION);  
  Serial.println("XIAO_M0");
  Serial.print("DAC Pin: ");Serial.print(DAC_PIN);
  Serial.print("  Resolution: ");Serial.println(DAC_RESOLUTION);
  return &Wire;
}
#endif
