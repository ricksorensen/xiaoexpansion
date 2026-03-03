# Gemini AI Context — XIAO Expansion Board Project

**Last updated:** 2026-03-02 (Added ESP32-C6 Air Quality Monitor)

## Project Overview

PlatformIO/Arduino project driving the **Seeed Studio XIAO Expansion Board** across multiple XIAO microcontroller variants. The expansion board includes:
- SSD1306 128×64 OLED display (I2C)
- PCF8563 RTC (I2C)
- Passive buzzer
- User button
- ADC input
- DAC output (SAMD21 only)

## File Structure

```
doexpansion/
├── platformio.ini          # Build environments for all boards
├── src/
│   ├── drivemany.cpp       # Main application (display, RTC, buzzer, ADC, button)
│   ├── drivegps.cpp        # GPS variant (adds TinyGPSPlus on Serial1)
│   ├── airquality.cpp      # ESP32-C6 Air Quality Monitor (DHT11, SGP41, SPA06, HM3301 with WebUI & MQTT)
│   ├── playsong.cpp        # Buzzer melody player (portable, no board-specific code)
│   ├── playsong.h          # Melody types/declarations
│   ├── chkdisp.cpp         # Test: font/display checker (standalone)
│   ├── rtc_seeed.cpp       # Test: SAMD21-only internal RTC (standalone)
│   └── testi2c.cpp         # Test: I2C bus scanner (standalone)
├── tools/
│   ├── dosine.py           # Sine wave generator script
│   └── main.py
├── README.md               # Board status, pin table, usage
├── notes                   # Detailed pin mappings per chip variant
├── tryit.txt               # Testing notes
└── doit                    # Bash build-all script
```

## Multi-Board Architecture

Code uses `#ifdef` branching based on PlatformIO-injected defines:

| Define | Board |
|---|---|
| `ARDUINO_SEEED_XIAO_M0` | XIAO SAMD21 |
| `ARDUINO_XIAO_ESP32C3` | XIAO ESP32-C3 |
| `ARDUINO_RASPBERRY_PI_PICO` | Raspberry Pi Pico (mbed) |
| `ARDUINO_SEEED_XIAO_RP2040` | XIAO RP2040 |
| `ARDUINO_Seeed_XIAO_nRF52840_Sense` | XIAO nRF52840 (Adafruit core) |
| `SEEED_XIAO_NRF52840_SENSE` | XIAO nRF52840 (mbed core) |

### Key per-board differences:
- **LED_PIN**: SAMD21/nRF52840 use `LED_BUILTIN`, RP2040 uses `17`, ESP32-C3 has none (`-1`)
- **DAC**: Only SAMD21 has a true DAC (pin A0, 10-bit)
- **ADC resolution**: All 12-bit; ESP32-C3 can't change resolution via `analogReadResolution()`
- **Interrupts**: ESP32-C3 ISR needs `IRAM_ATTR`; `FALLING` value differs across platforms
- **I2C**: All use default `Wire.begin()` for expansion board; RP2040 Pico had issues with different default pins

## Libraries
- `olikraus/U8g2` — OLED display driver
- `Bill2462/PCF8563-Arduino-Library` — RTC driver
- `mikalhart/TinyGPSPlus` — GPS parsing (drivegps only)

## Current Board Status (as of 2026-02-25)
- ✅ XIAO SAMD21: Full functionality
- ✅ XIAO ESP32-C3: Working (no LED, no DAC, ADC maxes at ~2.5V)
- ✅ XIAO RP2040: Working (no DAC)
- ✅ XIAO nRF52840 Sense: Working (both cores)
- ⚠️ Raspberry Pi Pico (mbed): I2C issues, no display
- ✅ XIAO ESP32-C6: Working on hardware (RGB NeoPixel on GPIO 8, SD card tested on D2, no DAC)

## Completed
- **XIAO ESP32-C6** support added (2026-02-25)
  - Uses Seeed platform: `https://github.com/Seeed-Studio/platform-seeedboards.git`
  - Requires `platform_packages` override for toolchain v14.2.0
  - Pin names are `D0`–`D3` (not `A0`–`A3` like C3)
  - Python deps: `intelhex`, `rich_click` needed for esptool v5
- **ESP32-C6 Air Quality Monitor** (`airquality_c6` env)
  - Uses `TCA9548A` multiplexer for I2C (Pa.HUB)
  - Sensors: `SGP41` (Voc/NOx), `SPA06-003` (Pressure/Temp), `HM3301` (PM2.5), `DHT11`
  - Features: WiFiManager, WebUI, Home Assistant MQTT Discovery
  - Due to `esptool` argument changes on C6, standard `platformio-build.py` had to be modified to remove `--flash-mode` / `--flash-freq` / `--flash-size` arguments.

## Notes for AI Assistants
- When modifying board support, update ALL of: `drivemany.cpp`, `drivegps.cpp`, `airquality.cpp`, `platformio.ini`, `README.md`, `notes`, and the `doit` script
- The `build_src_filter` in `[env]` excludes test files; only `drivemany.cpp` and `playsong.cpp/h` compile by default
- Pin names (A0–A3) map to different GPIO numbers per board — see `notes` file for full mappings
- **ESP32-C6 pin naming**: The Seeed C6 Arduino core does NOT define `A0`–`A3`. Use `D0`–`D3` instead. This is a key difference from ESP32-C3.
- **ESP32-C6 LED & SD Card**: The C6 has an RGB NeoPixel on GPIO 8 (use `neopixelWrite()`). The SD card CS pin is D2, which conflicts with `ADC_PIN` if both are used simultaneously.
- ESP32 boards need `IRAM_ATTR` on ISR functions
- The `while(!Serial);` in setup blocks forever if no terminal is connected (known issue on ESP32)
