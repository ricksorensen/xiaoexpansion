#if defined(ARDUINO_XIAO_ESP32C3)

#include <Arduino.h>
#include <SSD1306.h>

#define GPSSerial Serial1
#define BUTTON_PIN D1              // D1 not defined for XIAO_M0 ???
#define LED_PIN -1
#define BUZZER_PIN D3              //D3 didn't work for SAMD
#define DAC_PIN -1          // only on SAMD
#define ADC_PIN A2
#define MYADCRESOLUTION 12


inline static TwoWire* mcusetup(void) {
  Wire.begin(6,7);
  Wire.setClock(400000);
  //analogReadResolution(MYADCRESOLUTION);
  // RX 20, TX 21
  GPSSerial.begin(9600, SERIAL_8N1, 20, 21);
  Serial.println("XIAO_ESP32C3");
  return &Wire;
}
#endif
