#![allow(dead_code)]
pub mod edge_detect;
pub mod xbox;

use super::{
    drive::{self, Gear},
    superstructure, Subsystem,
};
use crate::cheesy_drive::CheesyDrive;
use crate::config::{superstructure::elevator, SUBSYSTEM_SLEEP_TIME};
use crossbeam_channel::Sender;
use std::thread;

type Drive = Sender<drive::Instruction>;
type Superstructure = Sender<superstructure::Instruction>;

#[allow(dead_code)]
struct Controller<T: Controls> {
    controls: T,
    cheesy: CheesyDrive,
    drive: Drive,
    superstructure: Superstructure,
}

impl<T: Controls> Controller<T> {
    fn new(controls: T, drive: Drive, superstructure: Superstructure) -> Self {
        Self {
            controls,
            cheesy: CheesyDrive::new(),
            drive,
            superstructure,
        }
    }
}

impl<T: Controls> Subsystem for Controller<T> {
    fn run(mut self) {
        let mut controls = self.controls;
        loop {
            thread::sleep(SUBSYSTEM_SLEEP_TIME);

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

            // SUPERSTRUCTURE
            if controls.abort_ball_intake() {
                self.superstructure
                    .send(superstructure::Instruction::AbortBallIntake)
                    .expect("Channel disconnected: ");
            } else if controls.begin_ball_intake() {
                self.superstructure
                    .send(superstructure::Instruction::BeginBallIntake)
                    .expect("Channel disconnected: ");
            } else if controls.outtake_ball() {
                self.superstructure
                    .send(superstructure::Instruction::BallOuttake)
                    .expect("Channel disconnected: ");
            }

            if controls.intake_hatch() {
                self.superstructure
                    .send(superstructure::Instruction::HatchIntake)
                    .expect("Channel disconnected: ");
            } else if controls.outtake_hatch() {
                self.superstructure
                    .send(superstructure::Instruction::HatchOuttake)
                    .expect("Channel disconnected: ");
            }

            if controls.elevator_manual_override() {
                let level = controls.elevator_override_level() * elevator::MAX;
                self.superstructure
                    .send(superstructure::Instruction::SetElevatorHeight(level))
                    .expect("Channel disconnected: ");
            } else if controls.elevator_low() {
                self.superstructure
                    .send(superstructure::Instruction::SetElevatorHeight(
                        elevator::LOW,
                    ))
                    .expect("Channel disconnected: ");
            } else if controls.elevator_mid_low() {
                self.superstructure
                    .send(superstructure::Instruction::SetElevatorHeight(
                        elevator::MID_LOW,
                    ))
                    .expect("Channel disconnected: ");
            } else if controls.elevator_mid_high() {
                self.superstructure
                    .send(superstructure::Instruction::SetElevatorHeight(
                        elevator::MID_HIGH,
                    ))
                    .expect("Channel disconnected: ");
            } else if controls.elevator_high() {
                self.superstructure
                    .send(superstructure::Instruction::SetElevatorHeight(
                        elevator::HIGH,
                    ))
                    .expect("Channel disconnected: ");
            }
        }
    }
}

pub trait Controls {
    fn throttle(&mut self) -> f64;
    fn wheel(&mut self) -> f64;
    fn low_gear(&mut self) -> bool;
    fn quick_turn(&mut self) -> bool;
    fn begin_ball_intake(&mut self) -> bool;
    fn abort_ball_intake(&mut self) -> bool;
    fn outtake_ball(&mut self) -> bool;
    fn intake_hatch(&mut self) -> bool;
    fn outtake_hatch(&mut self) -> bool;
    fn elevator_low(&mut self) -> bool;
    fn elevator_mid_low(&mut self) -> bool;
    fn elevator_mid_high(&mut self) -> bool;
    fn elevator_high(&mut self) -> bool;
    fn elevator_manual_override(&mut self) -> bool;
    // Must be between 0 and 1
    fn elevator_override_level(&mut self) -> f64;
}
