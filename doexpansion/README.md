Update: 20251210



Basic usage:
`pio run -e seeed_xiao -t upload`
note that pio does not seem to upload to nrf when plugged on expansion board.


Known `platformio` build environments (see `platformio.ini` for details):

- `seeed_xiao` for the Seeed XIAO SAMD21
- `pico` for the Raspberry Pi PICO (notionally the Seeed SAMD XIAO RP2040) with mbed, standard arduino tools
needs significant adaptation for XIAO RP2040 to get I2 working
- `seeed_xiao_esp32c3` for the Seeed XIAO ESP32C3
- `seeed_xiao_esp32c3_v5`for the Seeed XIAO ESP32C3 using

    `framework=espidf` to try and use IDF v5.2.x.  20240811 not working yet. 
- `pico_future` for the Seeed XIAO RP2040 using the `earlephilhower` toolset
preferred option for XIAO 2040
- `sense_future` for the Seeed XIAO NRF52840 Sense- with mbed
- `sense_future_adafruit` for the Seeed XIAO NRF52840 Sense, bare metal

Plus some others for the curious.

Implementation notes:

1. Buzzer is driven in source code directly ... simple blocking PWM signal generation.
2. Code is branched based on defines passed to the compiler based on selected environment:
   * `ARDUINO_SEEED_XIAO_M0`               default seeed_xiao
   * `ARDUINO_XIAO_ESP32C3`                default seeed_xiao_esp32c3
   * `ARDUINO_RASPBERRY_PI_PICO`           default pico build (official pi, mbed)
   * `ARDUINO_SEEED_XIAO_RP2040`           rp2040 earlephilhower tools
   * `ARDUINO_Seeed_XIAO_nRF52840_Sense`  arduino IDE, platformio adafruitnrf
   * `SEEED_XIAO_NRF52840_SENSE`           platformio, mbed nrf
3. Since currently (20230228) GPS/Network time is not being used, the PCF8563 time is arbitrarily set. 
4. Arduino libraries used are

```
lib_deps =
        olikraus/U8g2
        https://github.com/Bill2462/PCF8563-Arduino-Library
```
5. GPS is not currently used


Pins using arduino/platformio naming. 
+ xiao is my pin numbering CCW from 1-14 starting in the upper left at USB connector.  (5V is pin 14)

| Function | xiao | SAMD21      | RP2040 | RP2035 | NRF52840    | ESP32C3 | ESP32S3 |
| :---         | ---  | ---         | ---    | ---    | ---         | ---     | ---     |
| Buzzer   | 4    | A3,PA11         |        |        |             |         |         |
| I2C      | 5,6  | Default     |        |        |             |         |         |
| Button   | 2    | A1,PA4          |        |        |             |         |         |
| ADC      | 3    | A2,PA10          |        |        |             |         |         |
| DAC      | 1    | A0,PA2          | -      | -      | -           | -       | -       |
| UART     | 7,8  | PB8,PB9           | -      | -      | -           | -       | -       |
| CTRL     | 11   | PA6           | -      | -      | -           | -       | -       |
| LED      |      | LED_BUILTIN | 17     | 17     | LED_BUILTIN | -       | -       | 

End of table


 + seeed_xiao: works  led,buzzer/sound, display, pfc clock, adc, dac 
 + pico: led, buzzer works but is running all the time, display not updating  I2C problem.  No output on usb/serial
 + pico_future: works led,buzzer/sound, display, pfc clock, adc, (no dac)
 + seeed_xiao_esp32c3: works (no led),buzzer/sound, display, pfc clock, adc, (no dac)
 + sense_future: works  led,buzzer/sound, display, pfc clock, adc,(no dac) 
+  sense_future_adafruit: works  led,buzzer/sound, display, pfc clock, adc,(no dac) 
