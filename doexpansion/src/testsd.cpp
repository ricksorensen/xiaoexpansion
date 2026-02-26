// testsd.cpp -- SD card test for XIAO Expansion Board
// The expansion board's SD card CS is connected to D2
// This is a standalone test sketch (not part of drivemany/drivegps)
//
// Insert a MicroSD card, flash this, and open serial monitor at 115200.
// It will write "testing 1, 2, 3." to /test.txt and read it back.

#include <SPI.h>
#include <SD.h>
#include "FS.h"

#define SD_CS_PIN D2   // Expansion board SD card CS is on D2

File myFile;

void setup() {
  Serial.begin(115200);
  delay(3000);  // give time to open serial monitor (avoids while(!Serial) blocking)
  Serial.println("=== SD Card Test ===");
  Serial.print("CS Pin: D2 (GPIO ");
  Serial.print(SD_CS_PIN);
  Serial.println(")");

  pinMode(SD_CS_PIN, OUTPUT);

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("FAILED!");
    Serial.println("Check: is a MicroSD card inserted?");
    return;
  }
  Serial.println("OK.");

  // Print card info if available
  Serial.print("Card type: ");
  switch (SD.cardType()) {
    case CARD_MMC:  Serial.println("MMC");  break;
    case CARD_SD:   Serial.println("SD");   break;
    case CARD_SDHC: Serial.println("SDHC"); break;
    default:        Serial.println("Unknown"); break;
  }
  Serial.print("Card size: ");
  Serial.print(SD.cardSize() / (1024 * 1024));
  Serial.println(" MB");
  Serial.print("Used space: ");
  Serial.print(SD.usedBytes() / 1024);
  Serial.println(" KB");

  // Write test file
  myFile = SD.open("/test.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to /test.txt...");
    myFile.println("testing 1, 2, 3.");
    myFile.println("XIAO ESP32-C6 SD card test successful!");
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("ERROR: could not open /test.txt for writing");
    return;
  }

  // Read test file back
  myFile = SD.open("/test.txt");
  if (myFile) {
    Serial.println("--- Reading /test.txt ---");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    Serial.println("--- End of file ---");
    myFile.close();
  } else {
    Serial.println("ERROR: could not open /test.txt for reading");
  }

  Serial.println("=== SD Card Test Complete ===");
}

void loop() {
  // nothing — test runs once in setup
}
