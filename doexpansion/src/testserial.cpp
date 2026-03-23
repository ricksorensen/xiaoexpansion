#include <Arduino.h>
#include <<Adafruit_TinyUSB.h>
void setup() {
  Serial1.begin(9600);
  Serial.begin(115200);
}

void loop() {
  if (Serial1.available()) {
    Serial.print(Serial1.read());
  }
}
