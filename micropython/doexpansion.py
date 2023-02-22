import machine
import time

# import buzzsong
import buzzdylan
import ssd1306
import pcfsimp

BUTTON_PIN = "A1_D1"
LED_PIN = "USER_LED"
BUZZER_PIN = "A3_D3"
DAC_PIN = "A0_D0"
ADC_PIN = "A2_D2"
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = "A10_D10"
SIGNAL_USEGPS = "A9_D9"

i2c = machine.I2C(0, scl=machine.Pin("A5_D5"), sda=machine.Pin("A4_D4"), freq=400000)
print(i2c)
print(i2c.scan())
display = ssd1306.SSD1306_I2C(128, 64, i2c, addr=60)
pcf = pcfsimp.PCF8563(i2c)
print(pcf.get())
rtc = machine.RTC()
print(rtc.datetime())

button = machine.Pin("A1_D1", machine.Pin.IN, machine.Pin.PULL_UP)
signalGPS = machine.Pin(SIGNAL_USEGPS, machine.Pin.IN, machine.Pin.PULL_UP)
signalBuzz = machine.Pin(SIGNAL_BUZZOFF, machine.Pin.IN, machine.Pin.PULL_UP)

led = machine.Pin("USER_LED", machine.Pin.OUT)
led.high()  # start off

buzzer = machine.Pin("A3_D3", machine.Pin.OUT)
buttonPush = False


def dobuzzer(pin):
    bz = machine.PWM(pin, freq=440, duty_u16=0)
    buzzdylan.doBuzzer(bz)
    bz.deinit()


def button_int(bpin):
    global buttonPush
    buttonPush = True


def setGPStime(pcf, rtc):
    return False


def settime(pcf, rtc):
    if (signalGPS.value() == 0) and setGPStime(pcf, rtc):
        print(" Time set with GPS")
    else:
        pcf.set(12, 10, 0, 2, 21, 2, 23)
        rtc.datetime((2023, 2, 21, 1, 13, 20, 1, 0))
        print(" Time set arbitrarily. RTC set 1 hour ahead")


button.irq(button_int, trigger=machine.Pin.IRQ_FALLING)

settime(pcf, rtc)


while True:
    led.value(button.value())
    # RTC: datetime
    #   year, month, date, dow (0 is monday?), hour, minute, second, microsec
    # PFC8563...
    #   hour, minute, second, dow (0 is sunday), date, month, year (add 2000)
    pcf_time = pcf.get()
    rtc_time = rtc.datetime()
    display.fill(0)
    display.text(
        "Date {:04d}-{:02d}-{:02d}".format(
            pcf_time[6] + 2000, pcf_time[5], pcf_time[4]
        ),
        0,
        0,
    )
    display.text(
        "Date {:04d}-{:02d}-{:02d}".format(rtc_time[0], rtc_time[1], rtc_time[2]), 0, 10
    )
    display.text(
        "Time {:02d}-{:02d}-{:02d}".format(pcf_time[0], pcf_time[1], pcf_time[2]), 0, 20
    )
    display.text(
        "Time {:02d}-{:02d}-{:02d}".format(rtc_time[4], rtc_time[5], rtc_time[6]), 0, 30
    )
    display.text("more to follow", 8, 45)
    display.show()
    if buttonPush:
        print("Button Pushed")
        buttonPush = False
        if signalBuzz.value() == 0:
            dobuzzer(buzzer)
    time.sleep(1)
