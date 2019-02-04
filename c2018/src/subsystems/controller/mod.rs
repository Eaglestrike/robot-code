#![allow(dead_code)]
pub mod edge_detect;
pub mod xbox;

use super::{
    drive::{self, Gear},
    Subsystem,
};
use crate::cheesy_drive::CheesyDrive;
use crossbeam_channel::Sender;

type Drive = Sender<drive::Instruction>;

#[allow(dead_code)]
struct Controller<T: Controls> {
    controls: T,
    cheesy: CheesyDrive,
    drive: Drive,
}

impl<T: Controls> Controller<T> {
    fn new(controls: T, drive: Drive) -> Self {
        Self {
            controls,
            cheesy: CheesyDrive::new(),
            drive,
        }
    }
}

impl<T: Controls> Subsystem for Controller<T> {
    fn run(mut self) {
        let controls = self.controls;

        // DRIVE
        let wheel = controls.wheel();
        let throttle = controls.throttle();
        let quick_turn = controls.quick_turn();
        let high_gear = !controls.low_gear();
        if high_gear {
            self.drive.send(drive::Instruction::GearShift(Gear::High))
        } else {
            self.drive.send(drive::Instruction::GearShift(Gear::Low))
        }
        .expect("Channel disconnected: ");

        let signal = self
            .cheesy
            .cheesy_drive(wheel, throttle, quick_turn, high_gear);
        self.drive
            .send(drive::Instruction::Percentage(signal.l, signal.r))
            .expect("Channel disconnected: ");
    }
}

pub trait Controls {
    fn throttle(&self) -> f64;
    fn wheel(&self) -> f64;
    fn low_gear(&self) -> bool;
    fn quick_turn(&self) -> bool;
    fn begin_ball_intake(&self) -> bool;
    fn abort_ball_intake(&self) -> bool;
    fn outtake_ball(&self) -> bool;
    fn intake_hatch(&self) -> bool;
    fn elevator_low(&self) -> bool;
    fn elevator_mid_low(&self) -> bool;
    fn elevator_mid_high(&self) -> bool;
    fn elevator_high(&self) -> bool;
    fn elevator_manual_override(&self) -> bool;
    fn elevator_override_level(&self) -> f64;
}
