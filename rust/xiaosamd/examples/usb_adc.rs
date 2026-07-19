#![no_std]
#![no_main]

//! USB CDC example for the Adafruit QT Py board. Demonstrates creating a USB
//! CDC ACM serial port accessible over the on-board USB-C connector.
//! for SAMD debug mode is too big marginally?  ACMX does not persist with restarts reliably
//!    for debug.  Use --release

use panic_halt as _;
use core::fmt::{self, Write};
use atsamd_hal::adc::AdcBuilder;

use usb_device::prelude::*;
use usbd_serial::SerialPort;
use usbd_serial::USB_CLASS_CDC;

use hal::usb::UsbBus;
use usb_device::bus::UsbBusAllocator;

//use hal::adc::{Accumulation, Adc, Prescaler, Resolution};
use hal::adc::{Accumulation, Prescaler};

use bsp::hal;
use bsp::pac;

use bsp::entry;
use hal::clock::GenericClockController;
// use hal::delay::Delay;
// use pac::{CorePeripherals, Peripherals};
use pac::Peripherals;
use xiaosamd as bsp;

// prepare for formatting strings


// 1. Create a wrapper that implements core::fmt::Write for a mutable byte slice
struct SliceWriter<'a> {
    buffer: &'a mut [u8],
    offset: usize,
}

impl<'a> Write for SliceWriter<'a> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        if self.offset + bytes.len() > self.buffer.len() {
            return Err(fmt::Error);
        }
        self.buffer[self.offset..self.offset + bytes.len()].copy_from_slice(bytes);
        self.offset += bytes.len();
        Ok(())
    }
}

// 2. The formatting and conversion function
pub fn format_to_bytes<'a>(
    buffer: &'a mut [u8],
    args: fmt::Arguments<'_>,
) -> Result<&'a [u8], fmt::Error> {
    let mut writer = SliceWriter { buffer, offset: 0 };
    writer.write_fmt(args)?;
    Ok(&writer.buffer[..writer.offset])
}
// end formatting strings


#[entry]
fn main() -> ! {
    // take returns value once then None.  unwrap checks for valid or panic.
    let mut peripherals = Peripherals::take().unwrap();   
    // let core = CorePeripherals::take().unwrap();
    let mut clocks = GenericClockController::with_internal_32kosc(
        peripherals.gclk,
        &mut peripherals.pm,
        &mut peripherals.sysctrl,
        &mut peripherals.nvmctrl,
    );
    // let mut delay = Delay::new(core.SYST, &mut clocks);

//  from qt_m0 lib.rs:
// split create struct that has
// .usb: Usb { dm, dp }
//       Usb.init(Usb,clocks,pm) returns busallocator
//     let gclk0 = clocks.gclk0();
//     let usb_clock = &clocks.usb(&gclk0).unwrap()
//     UsbBusAllocator::new(UsbBus::new(usb_clock,pm,pins.usb_dm,pins.usb_dp,peripherals.usb)

    let pins = bsp::Pins::new(peripherals.port);   // oringall .split();
    let gclk0 = clocks.gclk0();
    let adc_clock = clocks.adc(&gclk0).unwrap();
    let mut adc = AdcBuilder::new(Accumulation::single(atsamd_hal::adc::AdcResolution::_12))
        .with_clock_cycles_per_sample(5)
        .with_clock_divider(Prescaler::Div128)
        .with_vref(atsamd_hal::adc::Reference::Intvcc0)
        .enable(peripherals.adc, &mut peripherals.pm, &adc_clock)
        .unwrap();
    let mut adc_pin = pins.a2.into_alternate();
    
    let usb_clock = &clocks.usb(&gclk0).unwrap();
    let usb_bus = UsbBusAllocator::new(UsbBus::new(
        usb_clock,
	&mut peripherals.pm,
	pins.usb_dm,
	pins.usb_dp,
	peripherals.usb,
    ));
    

    // let usb_bus = pins
    //     .usb
    //     .init(peripherals.usb, &mut clocks, &mut peripherals.pm);

    let mut serial = SerialPort::new(&usb_bus);
    //.supports_remote_wakeup(true)     // allow this to wake up suspended host

    let mut usb_device = UsbDeviceBuilder::new(&usb_bus, UsbVidPid(0x239a, 0x00cb))
        .strings(&[StringDescriptors::new(LangID::EN)
            .manufacturer("Fake company")
            .product("Serial port")
            .serial_number("TEST")])
        .expect("Failed to set strings")
        .device_class(USB_CLASS_CDC)
        .build();

    let mut bbuf = [0u8;16];
    let mut rx_buf = [0u8;128];
    //let mut reso = 65000;
    //let mut count = 1_000_001;
    loop {
        // activate usb?
	if  usb_device.poll(&mut [&mut serial]) {
	    match serial.read(&mut rx_buf) {      // read but ignore
	        Ok(0) =>  {}
		Ok(_count) => {
                    let res = adc.read(&mut adc_pin);
		    let _ = serial.write(
	                format_to_bytes(&mut bbuf, format_args!("adc = {:04}\r\n",res)).unwrap()
	            );
		}
		Err(_e) => {}
	    }
         }
	 // delay.delay_ms(1_000u32);
    }
}
