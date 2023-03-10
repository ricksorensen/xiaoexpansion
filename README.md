# xiaoexpansion
Micropython, Platformio, and Arduino examples using Seeed XIAO expansion board for multiple MCUs in the Seeed family.  

Simple examples using the Seeed XIAO expansion board with RTC(external), buzzer, display, and button.  The compiled arduino code works with the XIAO SAMD21, EXP32C3, RP2040, and NRF52840 boards.
The arduino code is built and loaded with platformio, only tested once with Arduino IDE.  

The micropython code is a work in progress - tested with custom SAMD21 micropython build to help with small available RAM.  This version can use a GPS input (xxRMC message) for acquiring time if available.

Devices onboard the expansion board are:
- PCF8563  RTC using I2C
- SSD1306 OLED 128x64 using I2C
- Piezo Buzzer
- Push button

Every mcu has Serial IO and ADC capabilities, but only the SAMD21 has a DAC. 

The micropython implementation will try to use the Serial IO to read a GPS module if present. Both implementation will exercise the PCF8563, OLED, BUZZER, and push button.  ADC is used to read <3.3V on pin1/A0.  REPL input can be used to provide a value which will be sent to the DAC if present.

Arduino libraries used are 
```
lib_deps =
        olikraus/U8g2
        https://github.com/Bill2462/PCF8563-Arduino-Library
```
the micropython external modules used are
- `ssd1306` from `micropython-lib`
- `pcf8563` from `https://github.com/sshahryiar/Raspberry-Pi-PICO---RP2040/blob/main/DVK511%20-%20PCF8563%20RTCC/PCF8563.py` renamed as `pcfsimp.py`
