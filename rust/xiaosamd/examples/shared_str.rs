#![no_std]
#![no_main]

extern crate panic_halt;

use core::fmt::{self, Write};

use hal::{clock::GenericClockController, delay::Delay, prelude::*, time::Hertz};
use pac::{CorePeripherals, Peripherals};
use core::cell::RefCell;
use embedded_hal_bus::i2c::RefCellDevice;
use ssd1306::{prelude::*, I2CDisplayInterface, Ssd1306};
use pcf8563::*;

use bsp::{entry, hal, pac};
use xiao_i2c as bsp;

#[entry]
fn main() -> ! {

    //let mut raw_space = [0u8; 64];
    //let mut my_buffer = MiniBuffer { buf: &mut raw_space, cursor: 0 };
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
        year: 27, // 2021
        month: 4, // April
        weekday: 0, // Sunday
        day: 4, 
        hours: 16,
        minutes: 52,
        seconds: 00,
    };
    rtc.set_datetime(&now).unwrap();

    loop {
        display.clear().unwrap();

  	let time = rtc.get_datetime().unwrap();
	let strargs = format_args!("\nwhat xmin={:02} \n xsec={:03}\n", time.minutes, time.seconds);
        display
            .write_fmt(strargs)
            .unwrap();

        delay.delay_ms(1_000u32);
    }
}
