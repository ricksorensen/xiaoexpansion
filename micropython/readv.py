from machine import Pin, ADC
import time

state = False


def pincb(_):
    global state
    state = True


def readV(apin=4, spin=5, atten=3):
    global state
    vin = ADC(apin, atten=atten)
    sp = Pin(spin, Pin.IN)
    sp.irq(pincb, trigger=Pin.IRQ_FALLING)
    print(vin.read())
    ts = time.ticks_ms()
    while True:
        if state:
            dt = time.ticks_diff(time.ticks_ms(), ts)
            print(f"{dt},{atten},{vin.read()},{vin.read_u16()},{vin.read_uv()}")
            state = False
