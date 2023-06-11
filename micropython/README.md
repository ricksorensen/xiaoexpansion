Modules used:

- `machine`:
   + `machine.UART`
   + `machine.I2C`
   + `machine.Pin`
   + `machine.PWM`
   + `machine.ADC`
   + `machine.DAC` (only for samd21)
- `time`
- `sys`
- `gc`
- `framebuf` (compiled into firmware)
- `ssd1306` Derived from `framebuf` pulled from standard `micropython-lib`
- `select`  Used to query REPL for incoming data.  (compiled into firmware)
- `pcfsimp` (lower memory usage PCF8563 module)
- implementation
   + `expansion.py`     main program
   + `buzzrainbow.py`   play a song on buzzer
   + `birthday.py`      happy birthday song  
   + `querystdtin.py`   Query REPL for incoming data
   + `alltime.py`       get time from GPS (if avail) or PCF ( if set) and configure RTC
   + `xiaozzzz.py`      particulars for each XIAO version


Micropython implementation notes

Released version 1.19.1 1.20.0  or later

xiao samd21:

* The samd21 is limited in both RAM and Flash space.  The default Flash filesystem for micrpython 1.19.1 is 64K, leaving 256-64-8 = 184K for the firmware.
   * As the most resource limited MCU in this exercise it drove many of the implementation decisions.  The limited RAM means that garbage collection should be called occasionally to have enough RAM for the display buffer.
   * The SSD1306 driver is derived from `framebuf`, which is not part of the 1.19.1 release.  Including this in the default SEEED_XIAO (or in development branch SEED_XIAO_SAMD21) caused the firmware to exceed the 184K size.  To add this in  I created a linker file to reduce the file system to 48K.  This also a allowed me to add `select` (compiled in) and 'ssd1306' (frozen) modules to the firmware.

xiao RP2040, esp32c3:

These can use stock micropython 1.19.1 build with `framebuf` and `select` compiled in.

