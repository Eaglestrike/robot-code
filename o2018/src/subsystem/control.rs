// TODO: This file is a copy paste from Ferret! It will not compile and needs a lot of work!

pub enum Input {
    Velocity(f64, f64),
    Percentage(f64, f64),
    Curvature(f64, f64)
}

use crate::subsystem::drive::drive_commands::MoveCommand;
use crate::util::config::controls::*;

pub trait DriveController<T> {
    fn new() -> Self;
    fn eval(&mut self, inputs: T) -> MoveCommand;
    fn reset(&mut self) {}
}

pub struct CheesyDriveController(f64);

/// (turn, throttle, quick turn)
impl DriveController<(f64, f64, bool)> for CheesyDriveController {
    fn new() -> Self {
        CheesyDriveController(0.0)
    }

    fn eval(&mut self, inputs: (f64, f64, bool)) -> MoveCommand {
        let angular_power;
        if inputs.2 {
            if inputs.1.abs() < 0.2 {
                self.0 = 0.9 * self.0 + inputs.0 * 0.2;
            }
            angular_power = inputs.0;

            let mut right = inputs.1 - angular_power;
            let mut left = inputs.1 + angular_power;
            if left.abs() > 1.0 {
                right -= left;
                left = left.min(1.0).max(-1.0);
                right -= left;
            } else if left.abs() > 1.0 {
                left -= right;
                right = right.min(1.0).max(-1.0);
                left -= right;
            }
            match MoveCommand::from_percents(left, right) {
                Ok(v) => v,
                Err(e) => e.scale(),
            }
        } else {
            angular_power = inputs.1.abs() * inputs.0 * TURN_SENSITIVITY - self.0;
            self.0 = match self.0 {
                e if e > 1.0 => e - 1.0,
                e if e < -1.0 => e + 1.0,
                _ => 0.0,
            };

            match MoveCommand::from_percents(inputs.1 + angular_power, inputs.1 - angular_power) {
                Ok(v) => v,
                Err(e) => e.clip(),
            }
        }
    }

    fn reset(&mut self) {
        self.0 = 0.0
    }
}

pub struct ClassicDriveController;

/// (x axis, y axis)
impl DriveController<(f64, f64)> for ClassicDriveController {
    fn new() -> Self {
        ClassicDriveController
    }

    fn eval(&mut self, inputs: (f64, f64)) -> MoveCommand {
        match MoveCommand::from_percents((inputs.1 + inputs.0) / 2.0, (inputs.1 - inputs.0) / 2.0) {
            Ok(v) => v,
            Err(e) => e.clip(),
        }
    }
}

pub struct TankDriveController;

/// (left side, right side)
impl DriveController<(f64, f64)> for TankDriveController {
    fn new() -> Self {
        TankDriveController
    }

    fn eval(&mut self, inputs: (f64, f64)) -> MoveCommand {
        match MoveCommand::from_percents(inputs.0, inputs.1) {
            Ok(v) => v,
            Err(e) => e.clip(),
        }
    }
}
