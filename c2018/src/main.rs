#[macro_use]
extern crate debug_stub_derive;

mod cheesy_drive;
mod config;
mod subsystems;

use std::time;
use wpilib::ds::*;
use wpilib::RobotBase;

use bus::Bus;
use crossbeam_channel::unbounded;
use std::thread;
use subsystems::drive::*;
use subsystems::Subsystem;

fn main() {
    let base = RobotBase::new().unwrap();
    let ds = base.make_ds();

    let bus = Bus::new(0);
    let (drive_send, recv) = unbounded();
    thread::spawn(move || {
        let drive = Drive::new(bus, recv);
        println!("drive: {:?}", drive);
        drive.run();
    });

    let lj = JoystickPort::new(0).unwrap();
    let rj = JoystickPort::new(1).unwrap();
    let throttle_axis = JoystickAxis::new(1).unwrap();
    let wheel_axis = JoystickAxis::new(0).unwrap();

    drive_send.send(Instruction::GearShift(Gear::High)).unwrap();
    RobotBase::start_competition();
    let mut old = time::Instant::now();
    let mut cdrive = cheesy_drive::CheesyDrive::new();
    loop {
        ds.wait_for_data();
        let signal = cdrive.cheesy_drive(
            ds.stick_axis(lj, throttle_axis).unwrap_or(0.0).into(),
            ds.stick_axis(rj, wheel_axis).unwrap_or(0.0).into(),
            false,
            false,
        );
        drive_send
            .send(Instruction::Percentage(signal.l, signal.r))
            .unwrap();
        let new = time::Instant::now();
        println!("{}", (new - old).subsec_micros());
        old = new;
    }
}
