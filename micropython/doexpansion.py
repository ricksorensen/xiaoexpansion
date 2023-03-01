import machine
import time
import querystdin
import gc

# import buzzsong
import buzzrainbow as buzztune
import ssd1306
import pcfsimp
import alltime

BUTTON_PIN = "A1_D1"
LED_PIN = "USER_LED"
BUZZER_PIN = "A3_D3"
DAC_PIN = "A0_D0"
ADC_PIN = "A2_D2"
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = "A10_D10"
# SIGNAL_USEGPS = "A9_D9"

i2c = machine.I2C(0, scl=machine.Pin("A5_D5"), sda=machine.Pin("A4_D4"), freq=400000)
# print(i2c)
# print(i2c.scan())
display = ssd1306.SSD1306_I2C(128, 64, i2c, addr=60)
pcf = pcfsimp.PCF8563(i2c)
print("pcf: ", pcf.get())
rtc = machine.RTC()
print("rtc: ", rtc.datetime())

uart = machine.UART(
    4,
    rx=machine.Pin("A7_D7"),
    tx=machine.Pin("A6_D6"),
    baudrate=9600,
)
timedata = alltime.AllTime(uart, pcf, rtc)
button = machine.Pin("A1_D1", machine.Pin.IN, machine.Pin.PULL_UP)
signalBuzz = machine.Pin(SIGNAL_BUZZOFF, machine.Pin.IN, machine.Pin.PULL_UP)

led = machine.Pin("USER_LED", machine.Pin.OUT)
led.high()  # start off

buzzer = machine.Pin("A3_D3", machine.Pin.OUT)
buttonPush = False


def dobuzzer(pin):
    bz = machine.PWM(pin, freq=440, duty_u16=0)
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
# only for SAMD
dac = machine.DAC(0)
adc = machine.ADC(machine.Pin(ADC_PIN))
newvoltage = None
while True:
    gc.collect()
    if (cin := qin.kbin()) is not None:
        vline = qin.kbLine(cin)
        vin = getVoltage(vline, 1023)
        if vin is not None:
            newvoltage = vin
            if dac is not None:
                dac.write(newvoltage)
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
