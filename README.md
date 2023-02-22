# xiaoexpansion
Micropython, Platformio, and Arduino examples using Seeed XIAO expansion board for multiple MCUs

Simple examples using Seeed XIAO expansion board with RTC(external), buzzer, display, and button.  The compiled arduino code works with the XIAO SAMD21, EXP32C3, RP2040, and NRF52840 boards.
The arduino code is built and loaded with platformio, only tested once with Arduino IDE.  The micropython code is a work in progress - tested with custom SAMD21 build. 

Devices onboard the expansion board are:
- PCF8563  RTC using I2C
- SSD1306 OLED 128x64 using I2C
- Piezo Buzzer
- Push button
every board has ADC capabilities, and the SAMD21 has a DAC


Arduino libraries used are 
```
lib_deps =
        olikraus/U8g2
        https://github.com/Bill2462/PCF8563-Arduino-Library
```
the micropython external modules used are `ssd1306` from `micropython-lib`, `pcf8563` from `https://github.com/sshahryiar/Raspberry-Pi-PICO---RP2040/blob/main/DVK511%20-%20PCF8563%20RTCC/PCF8563.py`
