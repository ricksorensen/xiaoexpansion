#include <Arduino.h>
#include "Wire.h"

/*** modifications to get Wire status ***
Wire.h: Add declaration to TwoWire class
   // Debug RJS
   void dumpparams(void) ;
   // maybe getters?
   inline int8_t getSda(void) { return sda; }
   inline int8_t getScl(void) { return scl; }
   inline uint8_t getI2CNum(void) { return num; }

Wire.cpp:  define dumpparams.cpp (maybe a getter for sda, scl instead?
   // debug RJS
   void TwoWire::dumpparams(void) {
      Serial.println("WIRE params");
      Serial.println("   SDA, SCL: ");Serial.print(sda);Serial.print(" ");Serial.println(scl);
   }

*** ***/


void setup() {
  Serial.begin(115200);
  Wire.begin();
  //  Wire.begin(SDA, SCL);
}

void loop() {
  byte error, address;
  int nDevices = 0;

  delay(5000);

  Serial.println("Scanning for I2C devices ...");
  // Wire.dumpparams();   needs modification of Wire.h/Wire.cpp
  for(address = 0x01; address < 0x7f; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if(error != 2){
      // 5 is timoout, 2 is NAK 
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found");
  }
}

