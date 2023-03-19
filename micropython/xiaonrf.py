import machine

BUTTON_PIN = "P3"  # P0.03 for A1
# P0.26 is Red, P0.30 is Green, P0.06 is Blue
# P0.17 is charge LED (Green)

LED_PIN = "P6"
BUZZER_PIN = "P29"  # P0.29 for A3
DAC_PIN = None
ADC_PIN = "P28"  # P0.28 for A2
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = "P47"  # P1.15 10 for D10
SIGNAL_USEGPS = "P46"  # P1.14 for D9

# I2CSCL =     # P0.05
# I2CSDA =     # P0.04
i2c = machine.I2C(0, scl=machine.Pin("P5"), sda=machine.Pin("P4"))
#
# 20230310: nrf uart does not have any() ??? See RHH
#  TX = P1.11, RX=P1.12
# set tx high to prevent float
machine.Pin("P43", machine.Pin.OUT).high()
uart = machine.UART(
    0,
    baudrate=9600,
)
# uart = None

dac = None


# pin is instance of machine.Pin
def buzzpwm(pin):
    pwm = machine.PWM(pin, freq=440, duty=0)
    return pwm
