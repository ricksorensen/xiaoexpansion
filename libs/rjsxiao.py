from machine import I2C, Pin, UART, SPI

# for xiao 20230220
# _pinFunctions = {
#     "DAC": 0,    # "A0_D0"
#     "SCL": 5,    # "A5_D5"
#     "SDA": 4,    # "A4_D4"
#     "RXD": 6,    # "A6_D6"
#     "TXD": 7,    # "A7_D7"
#     "SCK": 8,    # "A8_D8"
#     "MISO": 9,    # "A9_D9"
#     "MOSI": 10,    # "A10_D10"
# }
_pinFunctions = {
    "DAC": "A0_D0",
    "SCL": "A5_D5",
    "SDA": "A4_D4",
    "TXD": "A6_D6",
    "RXD": "A7_D7",
    "SCK": "A8_D8",
    "MISO": "A9_D9",
    "MOSI": "A10_D10",
}


# micropython does not support docstrings by default
#   see py/mpconfig.h: MICROPY_ENABLE_DOC_STRING
def help():
    print("XIAO Terminal utility functions")
    print("  I2CX(freq=400000): hardware I2C on sercom0 default xiao pins")
    print("  UARTX(baud=9600): hardware on sercom4 UART default xiao pins")
    print(
        "  SPIX(baud=9600,miso='MISO'): hardware SPI on sercom0 UART default xiao pins"
    )
    print("  functionPin(pinfunction): get uPy pin name  for special signals")
    print("                            eg: functionPin('SCL')->'A5_D5'")


def functionPin(pinfunction):
    return _pinFunctions.get(pinfunction, None)


def I2CX(freq=400000):
    # Pin5=scl, pin4=sda
    return I2C(
        0, scl=Pin(_pinFunctions["SCL"]), sda=Pin(_pinFunctions["SDA"]), freq=freq
    )


# SERCOM4: PB8, PB9
# default XIAO (rx=a6, tx=a7)
def UARTX(baud=9600):
    return UART(
        4, rx=Pin(_pinFunctions["RXD"]), tx=Pin(_pinFunctions["TXD"]), baudrate=baud
    )


def SPIX(baud=24_000_000, miso="MISO"):
    s = SPI(
        0,
        sck=Pin(_pinFunctions["SCK"]),
        mosi=Pin(_pinFunctions["MOSI"]),
        miso=Pin(_pinFunctions[miso]),
    )
    s.init(baudrate=baud)
    return s


# if I put del modules, flymake goes crazy
