"""Trinkey M0:
pin01: VBAT                 pin10: VBUS (USB?)
pin02: GND                  pin09: PA08/D0
pin03: PA06/D4              pin08: PA02/DAC/AREF/D1
pin04: PA07/D3              pin07: PA09/D2
pin05: RESET                pin06: 3.3V

DotStar: CLK:PA01, Data:PA00,
LED: PA10

"""

import machine
from math import sin, pi
import time


class simpleDotStar:
    # create the SPI object. miso can be any unused pin.
    # trinket M0 ds_clk=PA01, ds_data=PA00
    #            PA05 unused
    def __init__(
        self,
        clk=machine.Pin("PA01"),
        data=machine.Pin("PA00"),
        unused=machine.Pin("PA05"),
        turnoff=False,
    ):
        self._ds = machine.SoftSPI(sck=clk, mosi=data, miso=unused)
        if turnoff:
            self.write()  # turn off

    def write(self, *, red=0, green=0, blue=0):
        self._ds.write(b"\x00\x00\x00\x00\xff")
        self._ds.write(bytearray((blue, green, red)))
        self._ds.write(b"\xff\xff\xff")

    def deinit(self):
        self._ds.deinit()

    def init(self):
        self._ds.init()


def dotstarOff():
    ds = simpleDotStar(turnoff=True)
    ds.deinit()


def doSine(per=100, dt=0.01, ncycles=None):
    dotstar = simpleDotStar(turnoff=True)  # default class, turn off
    dotstar.deinit()
    da = machine.DAC(0)
    ct = 0
    while (not ncycles) or ct < ncycles:
        for i in range(per):
            v = int(0.5 + ((1 + sin(2 * pi * i / per)) * 512))
            if v > 1023:
                v = 1023
            # print(v)
            da.write(v)
            time.sleep(dt)
        ct = ct + 1
    da.write(0)


vtest = [0, 100, 128, 236, 256, 300, 320, 330, 400, 410, 512, 756, 800, 900, 1000, 1023]


def doVolts(vlist, dt=0.01, sigp="A2_D2", ncycles=None):
    da = machine.DAC(0)
    sp = machine.Pin(sigp, machine.Pin.OUT)
    sp.high()
    da.write(0)
    ct = 0
    ts = time.ticks_ms()
    while (not ncycles) or ct < ncycles:
        for v in vlist:
            da.write(v)
            print(time.ticks_diff(time.ticks_ms(), ts), ",", v)
            time.sleep(dt)
            sp.low()
            time.sleep(0.001)
            sp.high()
            da.write(0)
            time.sleep(dt)
        ct = ct + 1
    print("Done")
