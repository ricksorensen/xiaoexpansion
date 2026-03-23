#if defined(ARDUINO_Seeed_XIAO_nRF52840_Sense) || defined(SEEED_XIAO_NRF52840_SENSE)

#include <Arduino.h>

// Serial does not seem to work with bare-metal, needs mbed
#define GPSSerial Serial1
#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN LED_BUILTIN
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN -1          // only on SAMD
#define ADC_PIN A2
#define MYADCRESOLUTION 12

// use default I2C
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

inline static void mcusetup(void) {
  GPSSerial.begin(9600);
  //analogReadResolution(MYADCRESOLUTION);
#if defined(SEEED_XIAO_NRF52840_SENSE) 
  Serial.println("XIAO_NRF52840_SENSE (mbed)");
#else
  Serial.println("ARDUINO_Seeed_XIAO_nRF52840_Sense adafruit core");
#endif
}
#endif
