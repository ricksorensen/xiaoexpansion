#if defined(ARDUINO_Seeed_XIAO_nRF52840_Sense) || defined(SEEED_XIAO_NRF52840_SENSE)

#include <Arduino.h>
#include <SSD1306.h>

// Serial does not seem to work with bare-metal, needs mbed
#define GPSSerial Serial1
#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN LED_BUILTIN
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN -1          // only on SAMD
#define ADC_PIN A2
#define MYADCRESOLUTION 12


inline static TwoWire* mcusetup(void) {
  Wire.begin();
  Wire.setClock(400000);
  GPSSerial.begin(9600);
  //analogReadResolution(MYADCRESOLUTION);
#if defined(SEEED_XIAO_NRF52840_SENSE) 
  Serial.println("XIAO_NRF52840_SENSE (mbed)");
#else
  Serial.println("ARDUINO_Seeed_XIAO_nRF52840_Sense adafruit core");
#endif
  return &Wire;
}
#endif
