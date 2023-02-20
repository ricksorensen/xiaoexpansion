#include <Arduino.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  while (!Serial) ;        // ESP32C3 Serial seems up if powered from USB with no terminal pgm connected
  delay(5000);
  Serial.println("setup: Starting up");
  u8x8.begin();
  Serial.println("u8x8 begin ... done");
  u8x8.setFlipMode(1);     // 180 deg rotation of framebuffer
}
static int font=0;
const char *fontname;
void loop() {
  if (font == 0) {
    // 8wide, 8 high? 
    u8x8.setFont(u8x8_font_chroma48medium8_r);   // choose a suitable font
    fontname="chroma48";
  } else if (font == 1) {
    // 8wide, 8 high?  .. extra space in width
    u8x8.setFont(u8x8_font_5x8_r);
    fontname="5x8";
  } else if (font == 2) {
    // 8wide, 13 high (double)
    u8x8.setFont(u8x8_font_8x13B_1x2_r);
    fontname="8x13B";
  } else if (font == 3) {
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
    fontname = "amstrad";
  } else if (font == 4) {
    u8x8.setFont(u8x8_font_7x14_1x2_f);
    fontname = "7x14";
  } else if (font == 5) {
    u8x8.setFont(u8x8_font_artossans8_r);
    fontname = "artossans8";
  } else {
    // large
    u8x8.setFont(u8x8_font_inr21_2x4_r);
    fontname = "in421";
  }
  u8x8.clear();
  // extra wraps to beginning
  u8x8.print("1a2b3c4d5e6f7g8gextra");
  u8x8.setCursor(0,1);
  u8x8.print("A0B9C8D7E6F5G4H3extra");
  u8x8.setCursor(0,3);
  u8x8.print(" Font index: ");u8x8.print(font);
  u8x8.setCursor(0,4);
  u8x8.print(fontname);
  delay(10000);
  font = (font + 1) % 7;
}
