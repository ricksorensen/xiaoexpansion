#.py.mpy:
#	mpy-cross -o $@ $< 

MAIN = boot.py
MYLIBS = buzzsong.mpy rjsxiao.mpy pcfsimp.mpy
SYSLIBS = ssd1306
RUN = doexpansion.py

%.mpy: libs/%.py
	mpy-cross -o $@ $<

upload: $(MAIN) $(MYLIBS)
	mpremote cp $(MAIN) :/
	mpremote cp $(MYLIBS) :/lib/

syslib:
	mpremote mip install $(SYSLIBS)

run: $(RUN)
	mpremote run $(RUN)

