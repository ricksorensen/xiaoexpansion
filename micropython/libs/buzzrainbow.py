# MicroPython / Seeed Wio Terminal / SAMD51
# Wio-Terminal-Buzzer.py - play a scale on the buzzer with PWM
# scruss, 2022-10
# -*- coding: utf-8 -*-

from time import sleep_ms

TEMPO = 120  # ms sleep to give beat

whitekeys = {
    "A3": 220.00,
    "B3": 246.94,
    "C4": 261.63,
    "D4": 293.66,
    "E4": 329.63,
    "F4": 349.23,
    "G4": 392.00,
    "A4": 440.00,
    "B4": 493.88,
    "C5": 523.25,
    "D5": 587.33,
    "E5": 659.25,
    "F5": 698.46,
    "G5": 783.99,
    "A5": 880.00,
    "B5": 987.77,
}
#  with white space as extend
# "G4G4A4G4C5B4  G4G4A4G4D5C5  G4G4G5E5C5B4A4  E5E5E5C5D5C5"
rainbow = [
    ("C4", 4),
    ("C5", 4),
    ("B5", 2),
    ("G5", 1),
    ("A5", 1),
    ("B5", 2),
    ("C5", 2),
    ("C4", 4),
    ("A4", 4),
    ("G4", 8),
    ("A3", 4),
    ("F4", 4),
    ("E4", 2),
    ("C4", 1),
    ("D4", 1),
    ("E4", 2),
    ("F4", 2),
    ("D4", 2),
    ("B3", 1),
    ("C4", 1),
    ("D4", 2),
    ("E4", 2),
    ("C4", 4),
]


def playnote(note, duration, buzzer):
    if note != "  ":
        buzzer.freq(int(whitekeys[note] + 0.5))
        buzzer.duty_u16(32767)  # 50% duty
    sleep_ms(duration)
    buzzer.duty_u16(0)  # 0% duty - silent


def doBuzzer(buzzer, tempo=TEMPO, pause=5):
    buzzer.duty_u16(0)
    for note_dur in rainbow:
        playnote(note_dur[0], note_dur[1] * tempo, buzzer)
        sleep_ms(5)


def domain():
    import machine

    bz = machine.PWM(machine.Pin("A3_D3"), freq=440, duty_u16=0)
    doBuzzer(bz, tempo=250)


if __name__ == "__main__":
    domain()
