#if defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_SEEED_XIAO_RP2040)
#include <Arduino.h>
#include <SSD1306.h>

#define GPSSerial Serial1
#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN LED_BUILTIN
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN -1              // only on SAMD
#define DAC_RESOLUTION 0          // only SAMD21
#define ADC_PIN A2
#define MYADCRESOLUTION 12


inline static TwoWire* mcusetup(void) {
  Wire.begin();
  Wire.setClock(400000);
  // RX 1, TX 0
  GPSSerial.begin(9600);
  pinMode(25, OUTPUT);digitalWrite(25, 1);  // turn off blue
  pinMode(16, OUTPUT);digitalWrite(16, 1);  // turn off green
  pinMode(LED_PIN,OUTPUT);                  // turn off red
  digitalWrite(LED_PIN, 1);
  analogWriteResolution(DAC_RESOLUTION);
  analogReadResolution(MYADCRESOLUTION);
#if defined(ARDUINO_RASPBERRY_PI_PICO)
  Serial.println("PICO");
#elif defined(ARDUINO_SEEED_XIAO_RP2040)
  Serial.println("XIAO_RP2040");
#endif
  return &Wire;
}
#endif
