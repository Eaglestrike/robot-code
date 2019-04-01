mod cheesy_drive;
#[allow(dead_code)]
mod config;
mod subsystems;

use crate::subsystems::Subsystem;
use wpilib::ds::*;
use wpilib::RobotBase;

use bus::Bus;
use crossbeam_channel::unbounded;
use std::env;
use std::thread;
use subsystems::controller::*;
use subsystems::drive::*;
use subsystems::superstructure::*;

pub trait OkPrint {
    fn ok_print(self);
}

impl<T, E: std::fmt::Debug> OkPrint for Result<T, E> {
    fn ok_print(self) {
        if let Err(e) = self {
            println!("ok_print at {}:{}:{}:{:?}", file!(), line!(), column!(), e)
        }
    }
}

fn main() {
    env::set_var("RUST_BACKTRACE", "1");
    let bus = Bus::new(0);
    let (drive_send, drive_recv) = unbounded();
    let (super_send, super_recv) = unbounded();
    let base = RobotBase::new().unwrap();

    // let comp = wpilib::pneumatics::Compressor::new().unwrap();
    // comp.stop();

    thread::Builder::new()
        .name("Drive".to_string())
        .spawn(move || {
            let drive = Drive::new(bus, drive_recv);
            println!("drive: {:#?}", drive);
            drive.run();
        })
        .unwrap();

    thread::Builder::new()
        .name("SStruct".to_string())
        .spawn(move || {
            // TODO log
            let sstruct = Superstructure::new(super_recv).unwrap();
            println!("sstruct: {:#?}", sstruct);
            sstruct.run();
        })
        .unwrap();

    let lj = JoystickPort::new(0).unwrap();
    let rj = JoystickPort::new(1).unwrap();
    let oi = JoystickPort::new(2).unwrap();
    let controls = StandardControls::new(base.make_ds(), lj, rj, oi).unwrap();

    RobotBase::start_competition();

    // NOTE: All new control bindings or functions should be added in subsystems/controller/mod.rs
    let controller = Controller::new(controls, drive_send, super_send, base.make_ds());
    println!("controller: {:#?}", controller);
    controller.run();
}
