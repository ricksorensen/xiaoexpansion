#.py.mpy:
#	mpy-cross -o $@ $< 

# specify base directory for install on device
#   MP_BASE=           default root
#            /flash eg flash mount
MP_BASE=

# specify port for micropython instance (using mpremote shortcut)
#   MP_PORT=         empty if default (first one mpremote finds)
#           a0       ttyACM0, ..
#           a1                ..
#set with `make MP_PORT=a0`
MP_PORT=

# is ssd1306 already installed?
#   0 if not installed.  
MP_SSD1306=0

# mcu:  options are samd, rp2, esp32, nrf, zephyr
MP_MCU=samd
MCULIB=xiao$(MP_MCU).mpy
$(info  "MCU import: $(MCULIB)")


MAIN = querystdin.mpy

MYLIBS = buzzrainbow.mpy  pcfsimp.mpy
ALLTIME = alltime.mpy
ALLTIME_NRF = alltime_nrf.mpy
ifeq "$(MP_MCU)" "nrf"
	MYLIBS += $(ALLTIME_NRF)
else
	MYLIBS += $(ALLTIME)
endif
ifeq "$(MP_MCU)" "zephyr"
	RUN = expansionzephyr.mpy
else
	RUN = expansion.mpy  # help out SAMD21 
endif

SYSLIBS = ssd1306    # assumed installed on mcu - either frozen or as lib



%.mpy: libs/%.py
	mpy-cross -o $@ $<

%.mpy: %.py
	mpy-cross -o $@ $<

# uses mpremote cp, which will overwrite files if they exist on mcu
upload: $(MAIN) $(MYLIBS) $(MCULIB) $(RUN)
ifeq "$(MP_SSD1306)" "0"
	echo "Run mip install of ssd1306"
	mpremote $(MP_PORT) mip --target $(MP_BASE)/lib install ssd1306
endif
	mpremote $(MP_PORT) cp $(MAIN) :$(MP_BASE)
	# assumes existence of :/lib/ on mcu
	mpremote $(MP_PORT) cp $(MYLIBS) :$(MP_BASE)/lib/
	mpremote $(MP_PORT) cp $(MCULIB) :$(MP_BASE)
	mpremote $(MP_PORT) cp $(RUN) :$(MP_BASE)

syslib:
	mpremote mip --target $(MP_BASE)/lib install $(SYSLIBS)

run: $(RUN)
	mpremote run $(RUN)

help:
	echo "Expansion board micropython tests"
	echo "upload:  default target, load python files to mcu"
	echo "         MP_PORT port to use default $(MP_PORT)"
	echo "         MP_MCU mcu to use (samd, rp2, esp32,nrf,zephyr) default $(MP_MCU)"
	echo "run: run expansion module"
##### RP2
# mpremote $(MP_PORT) --target lib install ssd1306
