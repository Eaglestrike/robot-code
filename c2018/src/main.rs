#[macro_use]
extern crate debug_stub_derive;

mod cheesy_drive;
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

#[macro_use]
extern crate std;

pub trait OkPrint {
    fn ok_print(self);
}

impl<T, E: std::fmt::Debug> OkPrint for Result<T, E> {
    fn ok_print(self) {
        match self {
            Err(e) => println!("ok_print at {}:{}:{}:{:?}", file!(), line!(), column!(), e),
            Ok(_) => (),
        }
    }
}

pub mod built_info {
    // The file has been placed there by the build script.
    include!(concat!(env!("OUT_DIR"), "/built.rs"));
}

fn main() {
    env::set_var("RUST_BACKTRACE", "1");
    println!(
        "=====BUILD INFO=====\nBuild Time: {}\nGit Hash: {:?}\nRustc Version: {}\nProfile & Opt Level: {} {}",
        built_info::BUILT_TIME_UTC,
        built_info::GIT_VERSION,
        built_info::RUSTC_VERSION,
        built_info::PROFILE,
        built_info::OPT_LEVEL
    );

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
        });

    thread::Builder::new()
        .name("SStruct".to_string())
        .spawn(move || {
            // TODO log
            let sstruct = Superstructure::new(super_recv).unwrap();
            println!("sstruct: {:#?}", sstruct);
            sstruct.run();
        });

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
