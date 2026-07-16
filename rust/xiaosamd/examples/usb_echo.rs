#![no_std]
#![no_main]

//! USB CDC example for the Adafruit QT Py board. Demonstrates creating a USB
//! CDC ACM serial port accessible over the on-board USB-C connector.

use panic_halt as _;

use usb_device::prelude::*;
use usbd_serial::SerialPort;
use usbd_serial::USB_CLASS_CDC;

use hal::usb::UsbBus;
use usb_device::bus::UsbBusAllocator;

use bsp::hal;
use bsp::pac;

use bsp::entry;
use hal::clock::GenericClockController;
use pac::Peripherals;
use xiaosamd as bsp;

#[entry]
fn main() -> ! {
    let mut peripherals = Peripherals::take().unwrap();
    let mut clocks = GenericClockController::with_internal_32kosc(
        peripherals.gclk,
        &mut peripherals.pm,
        &mut peripherals.sysctrl,
        &mut peripherals.nvmctrl,
    );

//  from qt_m0 lib.rs:
// split create struct that has
// .usb: Usb { dm, dp }
//       Usb.init(Usb,clocks,pm) returns busallocator
//     let gclk0 = clocks.gclk0();
//     let usb_clock = &clocks.usb(&gclk0).unwrap()
//     UsbBusAllocator::new(UsbBus::new(usb_clock,pm,pins.usb_dm,pins.usb_dp,peripherals.usb)

    let pins = bsp::Pins::new(peripherals.port);   // oringall .split();
    let gclk0 = clocks.gclk0();
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
    let mut usb_device = UsbDeviceBuilder::new(&usb_bus, UsbVidPid(0x239a, 0x00cb))
        .strings(&[StringDescriptors::new(LangID::EN)
            .manufacturer("Fake company")
            .product("Serial port")
            .serial_number("TEST")])
        .expect("Failed to set strings")
        .device_class(USB_CLASS_CDC)
        .build();

    loop {
        if !usb_device.poll(&mut [&mut serial]) {
            continue;
        }

        let mut buf = [0u8; 64];
        if let Ok(count) = serial.read(&mut buf) {
            let _ = serial.write(&buf[..count]);
        }
    }
}
