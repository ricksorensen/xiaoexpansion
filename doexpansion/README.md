Basic usage:
`pio run -e seeed_xiao -t upload`

Known `platformio` build environments (see `platformio.ini` for details):
- `seeed_xiao` for the Seeed XIAO SAMD21
- `pico` for the Raspberry Pi PICO (notionally the Seeed SAMD XIAO RP2040) with mbed, standard arduino tools
needs significant adaptation for XIAO RP2040 to get I2 working
- `seeed_xiao_esp32c3` for the Seeed XIAO ESP32C3
- `pico_future` for the Seeed XIAO RP2040 using the `earlephilhower` toolset
preferred option for XIAO 2040
- `sense_future` for the Seeed XIAO NRF52840 Sense- with mbed
- `sense_future_adafruit` for the Seeed XIAO NRF52840 Sense, bare metal

Plus some others for the curious.

Implementation notes:
1. Buzzer is driven in source code directly ... simple blocking PWM signal generation.
2. Code is branched based on defines passed to the compiler based on selected environment:
   * ARDUINO_SEEED_XIAO_M0              default seeed_xiao
   * ARDUINO_XIAO_ESP32C3               default seeed_xiao_esp32c3
   * ARDUINO_RASPBERRY_PI_PICO          default pico build (official pi, mbed)
   * ARDUINO_SEEED_XIAO_RP2040          rp2040 earlephilhower tools
   * ARDUINO_Seeed_XIAO_nRF52840_Sense  arduino IDE, platformio adafruitnrf
   * SEEED_XIAO_NRF52840_SENSE          platformio, mbed nrf
3. Since currently (20230228) GPS/Network time is not being used, the PCF8563 time is arbitrarily set. 

