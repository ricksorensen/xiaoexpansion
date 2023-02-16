import machine
from math import sin, pi
import time


def doSine(per=100, dt=0.01, ncycles=None):
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
