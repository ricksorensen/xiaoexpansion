#![no_std]
#![no_main]

extern crate panic_halt;

use core::fmt::{self, Write};

use atsamd_hal::adc::AdcBuilder;
use core::cell::RefCell;
use embedded_hal_bus::i2c::RefCellDevice;
use hal::{clock::GenericClockController, delay::Delay, prelude::*, time::Hertz};
use pac::{CorePeripherals, Peripherals};
use pcf8563::*;
use ssd1306::{I2CDisplayInterface, Ssd1306, prelude::*};
//use hal::adc::{Accumulation, Adc, Prescaler, Resolution};
use hal::adc::{Accumulation, Prescaler};

use bsp::{entry, hal, pac};
use xiaosamd as bsp;

#[entry]
fn main() -> ! {
    let mut peripherals = Peripherals::take().unwrap();
    let core = CorePeripherals::take().unwrap();
    let mut clocks = GenericClockController::with_internal_32kosc(
        peripherals.gclk,
        &mut peripherals.pm,
        &mut peripherals.sysctrl,
        &mut peripherals.nvmctrl,
    );
    let pins = bsp::Pins::new(peripherals.port);
    let mut delay = Delay::new(core.SYST, &mut clocks);
    let i2c = bsp::i2c_master(
        &mut clocks,
        Hertz::kHz(400),
        peripherals.sercom0,
        &mut peripherals.pm,
        pins.a4,
        pins.a5,
    );
    let i2c_bus = RefCell::new(i2c);
    let mut dispi2c = RefCellDevice::new(&i2c_bus);
    let mut clki2c = RefCellDevice::new(&i2c_bus);

    let interface = I2CDisplayInterface::new(dispi2c);
    let mut display =
        Ssd1306::new(interface, DisplaySize128x64, DisplayRotation::Rotate0).into_terminal_mode();
    display.init().unwrap();
    // display.flush().unwrap();  // flush not available when i2c from i2c_bus.

    let mut rtc = PCF8563::new(clki2c);
    let now = DateTime {
        year: 27,   // 2021
        month: 4,   // April
        weekday: 0, // Sunday
        day: 4,
        hours: 16,
        minutes: 52,
        seconds: 00,
    };
    rtc.set_datetime(&now).unwrap();

    let gclk0 = clocks.gclk0();
    let adc_clock = clocks.adc(&gclk0).unwrap();
    // adc references:
    //   Int1v=0 (1.0v ref) Intvcc0=1 (3.3/1.48) Intvcc1=2 (3.3/2)
    //                      Arefa=3 (ext PA03) Arefb=4 (ext)
    let mut adc = AdcBuilder::new(Accumulation::single(atsamd_hal::adc::AdcResolution::_12))
        .with_clock_cycles_per_sample(5)
        .with_clock_divider(Prescaler::Div128)
        .with_vref(atsamd_hal::adc::Reference::Intvcc1)
        .enable(peripherals.adc, &mut peripherals.pm, &adc_clock)
        .unwrap();
    let mut adc_pin = pins.a2.into_alternate();

    loop {
        display.clear().unwrap();

        let time = rtc.get_datetime().unwrap();
        let res = adc.read(&mut adc_pin);
        let strargs = format_args!(
            "  time={:02}:{:02}:{:02}\n  year={:04}\n v={:04}",
            time.hours, time.minutes, time.seconds, time.year, res
        );
        display.write_fmt(strargs).unwrap();

        delay.delay_ms(1_000u32);
    }
}
