import machine

BUTTON_PIN = 27
LED_PIN = "LED"
BUZZER_PIN = 29
DAC_PIN = None
ADC_PIN = 28
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = 3
SIGNAL_USEGPS = 4

# I2CSCL =  7
# I2CSDA = "6
i2c = machine.I2C(1, scl=machine.Pin(7), sda=machine.Pin(6), freq=400000)

# SERRX = 1
# SERTX = 0
uart = machine.UART(
    0,
    rx=machine.Pin(1),
    tx=machine.Pin(0),
    baudrate=9600,
)

dac = None


def buzzpwm(pin):
    pwm = machine.PWM(pin)
    pwm.duty_u16(0)
    return pwm
