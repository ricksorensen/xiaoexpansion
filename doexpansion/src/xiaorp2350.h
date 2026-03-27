#if defined(ARDUINO_SEEED_XIAO_RP2350)
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
#if not defined(MYWIRE)
#define MYWIRE Wire
#endif
  MYWIRE.begin();
  MYWIRE.setClock(400000);
  // RX 1, TX 0
  GPSSerial.begin(9600);
  pinMode(25, OUTPUT);digitalWrite(25, 1);  // turn off yellow
  //pinMode(16, OUTPUT);digitalWrite(16, 1);  // SPI1 MISO on xiao 2350
  pinMode(LED_PIN,OUTPUT);                  // turn off red
  digitalWrite(LED_PIN, 1);
  analogWriteResolution(DAC_RESOLUTION);
  analogReadResolution(MYADCRESOLUTION);
  Serial.print("XIAO_RP2350   LED_PIN ");Serial.println(LED_PIN);
  return &MYWIRE;
}
#endif
