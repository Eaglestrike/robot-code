#![allow(dead_code)]
pub mod edge_detect;
pub mod xbox;

use super::{
    drive::{self, Gear},
    superstructure, Subsystem,
};
use crate::cheesy_drive::CheesyDrive;
use crate::config::superstructure::elevator;
use crossbeam_channel::Sender;
use wpilib::ds::*;

type Drive = Sender<drive::Instruction>;
type Superstructure = Sender<superstructure::Instruction>;

#[allow(dead_code)]
#[derive(Debug)]
pub struct Controller<'a, T: Controls> {
    controls: T,
    cheesy: CheesyDrive,
    drive: Drive,
    superstructure: Superstructure,
    ds: DriverStation<'a>,
}

impl<'a, T: Controls> Controller<'a, T> {
    pub fn new(
        controls: T,
        drive: Drive,
        superstructure: Superstructure,
        ds: DriverStation<'a>,
    ) -> Self {
        Self {
            controls,
            cheesy: CheesyDrive::new(),
            drive,
            superstructure,
            ds,
        }
    }
}

impl<'a, T: Controls> Subsystem for Controller<'a, T> {
    fn run(mut self) {
        let mut controls = self.controls;
        loop {
            self.ds.wait_for_data();

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

#[derive(Debug)]
pub struct StandardControls<'a> {
    ds: DriverStation<'a>,
    left: JoystickPort,
    right: JoystickPort,
    oi: JoystickPort,
    x: JoystickAxis,
    y: JoystickAxis,
}

impl<'a> StandardControls<'a> {
    pub fn new(
        ds: DriverStation<'a>,
        left: JoystickPort,
        right: JoystickPort,
        oi: JoystickPort,
    ) -> Result<Self, JoystickError> {
        Ok(Self {
            ds,
            left,
            right,
            oi,
            x: JoystickAxis::new(0)?,
            y: JoystickAxis::new(1)?,
        })
    }
}
impl<'a> Controls for StandardControls<'a> {
    fn throttle(&mut self) -> f64 {
        get_axis(&self.ds, self.left, self.y).into()
    }
    fn wheel(&mut self) -> f64 {
        get_axis(&self.ds, self.right, self.x).into()
    }
    fn low_gear(&mut self) -> bool {
        false
    }
    fn quick_turn(&mut self) -> bool {
        get_button(&self.ds, self.right, 0)
    }
    //TODO: Bind these
    fn begin_ball_intake(&mut self) -> bool {
        false
    }
    fn abort_ball_intake(&mut self) -> bool {
        false
    }
    fn outtake_ball(&mut self) -> bool {
        false
    }
    fn intake_hatch(&mut self) -> bool {
        false
    }
    fn outtake_hatch(&mut self) -> bool {
        false
    }
    fn elevator_low(&mut self) -> bool {
        false
    }
    fn elevator_mid_low(&mut self) -> bool {
        false
    }
    fn elevator_mid_high(&mut self) -> bool {
        false
    }
    fn elevator_high(&mut self) -> bool {
        false
    }
    fn elevator_manual_override(&mut self) -> bool {
        false
    }
    // Must be between 0 and 1
    fn elevator_override_level(&mut self) -> f64 {
        0.0
    }
}
fn get_button(ds: &DriverStation<'_>, port: JoystickPort, num: u8) -> bool {
    ds.stick_button(port, num).unwrap_or(false)
}
fn get_axis(ds: &DriverStation<'_>, port: JoystickPort, axis: JoystickAxis) -> f32 {
    ds.stick_axis(port, axis).unwrap_or(0.0)
}
fn get_pov(ds: &DriverStation<'_>, port: JoystickPort, pov: JoystickPOV) -> i16 {
    ds.stick_pov(port, pov).unwrap_or(0)
}
