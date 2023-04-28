#.py.mpy:
#	mpy-cross -o $@ $< 

# specify port for micropython instance (using mpremote shortcut)
#   MP_PORT=         empty if default (first one mpremote finds)
#           a0       ttyACM0, ..
#           a1                ..
#set with `make MP_PORT=a0`
MP_PORT=

# is ssd1306 already installed?
#   0 if not installed.  
MP_SSD1306=1

# mcu:  options are samd, rp2, esp32, nrf
MP_MCU=samd
MCULIB=xiao$(MP_MCU).mpy
$(info  "MCU import: $(MCULIB)")


MAIN = boot.py querystdin.mpy

MYLIBS = buzzrainbow.mpy  pcfsimp.mpy
ALLTIME = alltime.mpy
ALLTIME_NRF = alltime_nrf.mpy
ifeq "$(MP_MCU)" "nrf"
	MYLIBS += $(ALLTIME_NRF)
else
	MYLIBS += $(ALLTIME)
endif
SYSLIBS = ssd1306    # assumed installed on mcu - either frozen or as lib


RUN = expansion.mpy  # help out SAMD21 

%.mpy: libs/%.py
	mpy-cross -o $@ $<

%.mpy: %.py
	mpy-cross -o $@ $<

# uses mpremote cp, which will overwrite files if they exist on mcu
upload: $(MAIN) $(MYLIBS) $(MCULIB) $(RUN)
ifeq "$(MP_SSD1306)" "0"
	echo "Run mip install of ssd1306"
	mpremote $(MP_PORT) mip --target lib install ssd1306
endif
	mpremote $(MP_PORT) cp $(MAIN) :/
	# assumes existence of :/lib/ on mcu
	mpremote $(MP_PORT) cp $(MYLIBS) :/lib/
	mpremote $(MP_PORT) cp $(MCULIB) :/
	mpremote $(MP_PORT) cp $(RUN) :/

syslib:
	mpremote mip install $(SYSLIBS)

run: $(RUN)
	mpremote run $(RUN)

##### RP2
# mpremote $(MP_PORT) --target lib install ssd1306
