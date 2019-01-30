mod config;
mod subsystems;

use bus::Bus;
use crossbeam_channel::unbounded;
use std::thread;
use subsystems::drive::*;
use subsystems::Subsystem;

fn main() {
    let bus = Bus::new(0);
    let (drive_send, recv) = unbounded();
    thread::spawn(move || {
        let drive = Drive::new(bus, recv);
        drive.run();
    });
    drive_send.send(Instruction::GearShift(Gear::High)).unwrap();
    println!("Hello, world!");
}
