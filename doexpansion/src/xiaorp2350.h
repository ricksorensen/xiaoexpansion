#if defined(ARDUINO_SEEED_XIAO_RP2350)
#include <Arduino.h>

#define GPSSerial Serial1
#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN LED_BUILTIN
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN -1              // only on SAMD
#define DAC_RESOLUTION 0          // only SAMD21
#define ADC_PIN A2
#define MYADCRESOLUTION 12

// use XIAO pinout
U8X8_SSD1306_128X64_NONAME_2ND_HW_I2C u8x8(/*clock=*/7u, /* data=*/ 6u, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
//U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display

inline static void mcusetup(void) {
  // RX 1, TX 0
  Serial1.begin(115200);
  pinMode(25, OUTPUT);digitalWrite(25, 1);  // turn off yellow
  //pinMode(16, OUTPUT);digitalWrite(16, 1);  // SPI1 MISO on xiao 2350
  pinMode(LED_PIN,OUTPUT);                  // turn off red
  digitalWrite(LED_PIN, 1);
  analogWriteResolution(DAC_RESOLUTION);
  analogReadResolution(MYADCRESOLUTION);
  Serial.print("XIAO_RP2350   LED_PIN ");Serial.println(LED_PIN);
}
#endif
