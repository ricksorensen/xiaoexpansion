import machine

BUTTON_PIN = ("gpio0", 3)  # P0.03 for A1
# P0.26 is Red, P0.30 is Green, P0.06 is Blue
# P0.17 is charge LED (Green)

LED_PIN = ("gpio0", 6)
BUZZER_PIN = ("gpio0", 29)  # P0.29 for A3
DAC_PIN = None
ADC_PIN = ("gpio0", 28)  # P0.28 for A2
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = ("gpio1", 15)  # P1.15 10 for D10
SIGNAL_USEGPS = ("gpio1", 14)  # P1.14 for D9

#  I2C(0) is internal i2c bus (imu)
#  I2C(1) is external, default xiao scl,sca pins
# I2CSCL =     # P0.05
# I2CSDA =     # P0.04
i2c = machine.I2C("i2c1")  # currently does not support arbitrary scl,sda assignment

#
# 20230310: nrf uart does not have any() ??? See RHH
#  TX = P1.11, RX=P1.12
# set tx high to prevent float
machine.Pin(("gpio1", 11), machine.Pin.OUT).on()
uart = machine.UART(
    "uart0",
    baudrate=9600,
)
# uart = None

dac = None
adc = machine.ADC(("adc", 4))


# pin is instance of machine.Pin
def buzzpwm(pin):
    # pwm = machine.PWM(pin, freq=440, duty_u16=0)
    pwm = machine.PWM(("pwm0", 1), freq=440, duty_u16=0)
    return pwm
