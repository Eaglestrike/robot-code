#![allow(dead_code)]
pub mod edge_detect;
pub mod xbox;

use super::{
    drive::{self, Gear},
    superstructure, Subsystem,
};
use crate::cheesy_drive::CheesyDrive;
use crate::config::{self, superstructure::elevator, SUBSYSTEM_SLEEP_TIME};
use crossbeam_channel::Sender;
use std::{f64, thread};
use wpilib::ds::*;

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

pub struct StandardControls<'a> {
    ds: &'a DriverStation<'a>,
    left: JoystickPort,
    right: JoystickPort,
    oi: JoystickPort,
    x: JoystickAxis,
    y: JoystickAxis,
}

impl<'a> StandardControls<'a> {
    fn new(
        ds: &'a DriverStation<'a>,
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
        adjust_throttle(band(get_axis(self.ds, self.left, self.y).into()))
    }
    fn wheel(&mut self) -> f64 {
        adjust_wheel(band(get_axis(self.ds, self.right, self.x).into()))
    }
    fn low_gear(&mut self) -> bool {
        get_button(self.ds, self.left, 1)
    }
    fn quick_turn(&mut self) -> bool {
        get_button(self.ds, self.right, 1)
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
#[allow(clippy::let_and_return)]
fn adjust_throttle(throttle: f64) -> f64 {
    use config::controls::THROTTLE_GROWTH;
    let denominator = (f64::consts::FRAC_PI_2 * THROTTLE_GROWTH).sin();
    let throttle = (f64::consts::FRAC_PI_2 * THROTTLE_GROWTH * throttle) / denominator;
    let throttle = (f64::consts::FRAC_PI_2 * THROTTLE_GROWTH * throttle) / denominator;
    throttle
}
#[allow(clippy::let_and_return)]
fn adjust_wheel(wheel: f64) -> f64 {
    use config::controls::WHEEL_GROWTH;
    let denominator = (f64::consts::FRAC_PI_2 * WHEEL_GROWTH).sin();
    let wheel = (f64::consts::FRAC_PI_2 * WHEEL_GROWTH * wheel) / denominator;
    let wheel = (f64::consts::FRAC_PI_2 * WHEEL_GROWTH * wheel) / denominator;
    let wheel = (f64::consts::FRAC_PI_2 * WHEEL_GROWTH * wheel) / denominator;
    wheel
}
fn band(val: f64) -> f64 {
    use config::controls::STANDARD_DEADBAND;
    if val.abs() > STANDARD_DEADBAND {
        val
    } else {
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
