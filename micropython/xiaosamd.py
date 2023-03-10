import machine

BUTTON_PIN = "A1_D1"
LED_PIN = "USER_LED"
BUZZER_PIN = "A3_D3"
DAC_PIN = "A0_D0"
ADC_PIN = "A2_D2"
MYADCRESOLUTION = 12

SIGNAL_BUZZOFF = "A10_D10"
SIGNAL_USEGPS = "A9_D9"

# I2CSCL = "A5_D5"
# I2CSDA = "A4_D4"
i2c = machine.I2C(0, scl=machine.Pin("A5_D5"), sda=machine.Pin("A4_D4"), freq=400000)

# SERRX = "A6_D6"
# SERTX = "A7_D7"
uart = machine.UART(
    4,
    rx=machine.Pin("A7_D7"),
    tx=machine.Pin("A6_D6"),
    baudrate=9600,
)

dac = machine.DAC(0)


def buzzpwm(pin):
    return machine.PWM(pin, freq=440, duty_u16=0)
