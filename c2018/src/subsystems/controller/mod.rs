#![allow(dead_code)]
pub mod edge_detect;
pub mod xbox;

use crossbeam_channel::Sender;
use super::{drive, Subsystem};
use crate::cheesy_drive::CheesyDrive;

type Drive = Sender<drive::Instruction>;

#[allow(dead_code)]
struct Controller<T: Controls> {
    controls: T,
    cheesy_drive: CheesyDrive,
    drive: Drive
}

impl<T: Controls> Controller<T> {
    fn new(controls: T, drive: Drive) -> Self {
        Self {
            controls,
            cheesy_drive: CheesyDrive::new(),
            drive
        }
    }
}

impl<T: Controls> Subsystem for Controller<T> {
    fn run(self) {

    }
}

pub trait Controls {
    fn throttle(&self) -> f64;
    fn wheel(&self) -> f64;
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
