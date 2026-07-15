#![no_std]
#![no_main]
use panic_halt as _;
use atsamd_hal as hal;
use cortex_m_rt::entry;
use hal::pac;
use hal::clock::GenericClockController;
use hal::usb::Usb;
use usb_device::device::{UsbDeviceBuilder, UsbVidPid};
use usbd_serial::{SerialPort, USB_CLASS_CDC};
use core::fmt::Write; // For using writeln!

#[entry]
fn main() -> ! {
    let mut pac = pac::Peripherals::take().unwrap();
    let mut clocks = GenericClockController::with_internal_32kosc(
        pac.gclk,
        &mut pac.pm,
        &mut pac.sysctrl,
        &mut pac.nvmctrl,
    );
    let pins = hal::pins::Pins::new(pac.port);
    // ... other peripheral setup ...

    // Setup USB
    let usb = Usb::new(
        &mut clocks,
        pac.pm,
        pac.sysctrl,
        pac.gclk,
        pac.usb,
        &mut pac.port,
        pins.pa24, // Use the correct D+ pin for XIAO SAMD21
        pins.pa25, // Use the correct D- pin
    );

    let mut serial = SerialPort::new(&usb);

    let mut usb_dev = UsbDeviceBuilder::new(&usb, UsbVidPid(0xdead, 0xbeef)) // Example VID/PID
    	.strings(&[usb_device::device::StringDescriptors::default()
		.product("XIAO SAMD21 Serial Port")
		.serial_number("42")])
	.unwrap()
        .device_class(USB_CLASS_CDC)
        .build();

    // Main loop
    loop {
        if !usb_dev.poll(&mut [&mut serial]) {
            continue;
        }

        // Read data from the host
        let mut buf = [0u8; 64];
        match serial.read(&mut buf) {
            Ok(count) => {
                // Process received data
                // Example: echo back the data
                serial.write(&buf[..count]).unwrap();
            }
            Err(usb_device::UsbError::WouldBlock) => {} // No data received
            Err(_) => {} // Other errors
        };

        // Write data to the host (example)
        // writeln!(&mut serial, "Hello, Rust!").unwrap(); // This requires custom write implementation for SerialPort
    }
}
