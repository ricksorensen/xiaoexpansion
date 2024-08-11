import machine

BUTTON_PIN = 3  # GPIO3 for A1
LED_PIN = None
BUZZER_PIN = 5  # GPIO5 for A3
DAC_PIN = None
ADC_PIN = 4  # GPIO4 for A2
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = 10  # GPIO 10 for D10
SIGNAL_USEGPS = 9  # GPIO 9 for D9

# I2CSCL =  7
# I2CSDA = "6
i2c = machine.I2C(0, scl=machine.Pin(7), sda=machine.Pin(6), freq=1000000)
#
#
uart = machine.UART(
    1,
    tx=21,  # pin number, not machine.Pin
    rx=20,  # pin number, not machine.Pin
    baudrate=9600,
)

dac = None


def buzzpwm(pin):
    pwm = machine.PWM(pin)
    pwm.duty_u16(0)
    return pwm


# ESP32C3 ADC has several options for attenuation
#   which affect the range
#   maximum range is attenuation 3 (11DB) for [0, 2.5V] approx range
#
adc = machine.ADC(ADC_PIN, atten=3)
