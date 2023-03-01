import machine
import time
import querystdin
import gc

# import buzzsong
import buzzrainbow as buzztune
import ssd1306
import pcfsimp
import alltime

import xiaosamd as mcu

# print(mcu.i2c)
# print(mcu.i2c.scan())
gc.collect()
print(gc.mem_free())
gc.collect()

display = ssd1306.SSD1306_I2C(128, 64, mcu.i2c, addr=60)
pcf = pcfsimp.PCF8563(mcu.i2c)
print("pcf: ", pcf.get())
rtc = machine.RTC()
print("rtc: ", rtc.datetime())

timedata = alltime.AllTime(mcu.uart, pcf, rtc)
button = machine.Pin(mcu.BUTTON_PIN, machine.Pin.IN, machine.Pin.PULL_UP)
signalBuzz = machine.Pin(mcu.SIGNAL_BUZZOFF, machine.Pin.IN, machine.Pin.PULL_UP)

led = machine.Pin(mcu.LED_PIN, machine.Pin.OUT)
led.high()  # start off

buzzer = machine.Pin(mcu.BUZZER_PIN, machine.Pin.OUT)
buttonPush = False


def dobuzzer(pin):
    bz = mcu.buzzpwm(pin)
    buzztune.doBuzzer(bz)
    bz.deinit()


def button_int(bpin):
    global buttonPush
    buttonPush = True


def getVoltage(strin, maxv):
    rv = None
    lstr = len(strin)
    for i0 in range(lstr):
        if strin[i0].isdigit():
            break
    if strin[i0].isdigit():
        for ix in range(i0, lstr):
            if not strin[ix].isdigit():
                break
        rv = int(strin[i0:ix])
        if rv > maxv:
            rv = maxv
    return rv


button.irq(button_int, trigger=machine.Pin.IRQ_FALLING)

qin = querystdin.StdinQuery()

adc = machine.ADC(machine.Pin(mcu.ADC_PIN))
newvoltage = None
print("Starting")
while True:
    gc.collect()
    if (cin := qin.kbin()) is not None:
        vline = qin.kbLine(cin)
        vin = getVoltage(vline, 1023)
        if vin is not None:
            newvoltage = vin
            if mcu.dac is not None:
                mcu.dac.write(newvoltage)
        print("newV: " + vline)
    led.value(button.value())
    # RTC: datetime
    #   year, month, date, dow (0 is monday?), hour, minute, second, microsec
    # PFC8563...
    #   hour, minute, second, dow (0 is sunday), date, month, year (add 2000)
    pcf_time = pcf.get()
    rtc_time = rtc.datetime()
    display.fill(0)
    display.text(
        "pDate {:04d}-{:02d}-{:02d}".format(
            pcf_time[6] + 2000, pcf_time[5], pcf_time[4]
        ),
        0,
        0,
    )
    display.text(
        "rDate {:04d}-{:02d}-{:02d}".format(rtc_time[0], rtc_time[1], rtc_time[2]),
        0,
        10,
    )
    display.text(
        "pTime {:02d}-{:02d}-{:02d}".format(pcf_time[0], pcf_time[1], pcf_time[2]),
        0,
        20,
    )
    display.text(
        "rTime {:02d}-{:02d}-{:02d}".format(rtc_time[4], rtc_time[5], rtc_time[6]),
        0,
        30,
    )
    if newvoltage is not None:
        display.text("vDAC: {:04d}".format(newvoltage), 0, 40)
    display.text("vADC: {:04d}".format(adc.read_u16()), 0, 50)
    display.show()
    if buttonPush:
        print("Button Pushed")
        buttonPush = False
        if signalBuzz.value() == 0:
            dobuzzer(buzzer)
    time.sleep(1)
