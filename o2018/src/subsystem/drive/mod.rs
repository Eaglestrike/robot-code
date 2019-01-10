use navx::AHRS;
use wpilib::encoder::{Encoder, EncodingType};
use wpilib::pneumatics::{Action, DoubleSolenoid};
use wpilib::pwm::PwmSpeedController;
use wpilib::spi::Port::MXP;

use crate::subsystem::drive::drive_side::{DriveSide, PwmDriveSide, TalonDriveSide};
use crate::util::config::drive::*;
use crate::util::config::drive::pwm::*;
use crate::util::talon_factory::*;

mod drive_side;

#[derive(Debug, PartialEq)]
pub struct Pose {
    pub x: f64,
    pub y: f64,
    pub heading: f64,
    pub velocity: f64,
    pub angular_velocity: f64,
    distance_accumulated: f64,
}

pub struct Drive<T: DriveSide> {
    left_drive_side: T,
    right_drive_side: T,
    ahrs: AHRS,
    gear_shifter: DoubleSolenoid,
}

impl<T: DriveSide> Drive<T> {
    /// Set the drive percent outputs
    pub fn set(&mut self, left: f64, right: f64) {
        self.left_drive_side.set_percent(left.min(1.0).max(-1.0));
        self.right_drive_side.set_percent(right.min(1.0).max(-1.0));
    }

    pub fn set_velocity(&mut self, left: f64, right: f64) {
        self.left_drive_side.set_velocity(left.min(MAX_VELOCITY).max(-MAX_VELOCITY));
        self.right_drive_side.set_velocity(right.min(MAX_VELOCITY).max(-MAX_VELOCITY));
    }

    #[cfg(gear_shifter)]
    pub fn set_gear(&mut self, state: Action) {
        self.gear_shifter
            .set(state)
            .expect("Unable to set gear shifter!")
    }

    /// Get the current gear the robot is in. If the gear shifter is not enabled, this function
    /// will return `Action::Off`.
    #[inline(always)]
    pub fn gear(&self) -> Action {
        if cfg!(gear_shifter) {
            Action::Off
        } else {
            self.gear_shifter
                .get()
                .expect("Unable to get gear shifter state!")
        }
    }
}

impl Drive<TalonDriveSide> {
    pub fn new() -> Self {
        Self {
            left_drive_side: TalonDriveSide::new(
                motor_type::CtreCim::create(LEFT_MASTER),
                motor_type::CtreCim::create(LEFT_SLAVE),
            ),
            right_drive_side: TalonDriveSide::new(
                motor_type::CtreCim::create(RIGHT_MASTER),
                motor_type::CtreCim::create(RIGHT_SLAVE),
            ),
            ahrs: AHRS::from_spi_minutiae(MXP, 500_000, 60),
            gear_shifter: DoubleSolenoid::new(shifter::LOW_GEAR_CHANNEL, shifter::HIGH_GEAR_CHANNEL)
                .expect("Unable to create gear shifter!"),
        }
    }
}

impl Drive<drive_side::PwmDriveSide> {
    pub fn new() -> Self {
        Self {
            left_drive_side: PwmDriveSide::new(
                DualPwm::new(LEFT_MASTER, LEFT_SLAVE, LEFT_ENCODER_A, LEFT_ENCODER_B),
                LEFT_K_VELOCITY,
                LEFT_K_ACCELERATION,
            ),
            right_drive_side: PwmDriveSide::new(
                DualPwm::new(RIGHT_MASTER, RIGHT_SLAVE, RIGHT_ENCODER_A, RIGHT_ENCODER_B),
                RIGHT_K_VELOCITY,
                RIGHT_K_ACCELERATION,
            ),
            ahrs: AHRS::from_spi_minutiae(MXP, 500_000, 60),
            gear_shifter: DoubleSolenoid::new(shifter::LOW_GEAR_CHANNEL, shifter::HIGH_GEAR_CHANNEL)
                .expect("Unable to create gear shifter!"),
        }
    }
}

pub struct DualPwm {
    master: PwmSpeedController,
    slave: PwmSpeedController,
    encoder: Encoder,
}

impl DualPwm {
    pub fn new(motor_a: i32, motor_b: i32, encoder_a: i32, encoder_b: i32) -> Self {
        DualPwm {
            master: PwmSpeedController::new_talon(motor_a).expect("Unable to create subsystem.drive pwm"),
            slave: PwmSpeedController::new_talon(motor_b).expect("Unable to create subsystem.drive pwm"),
            encoder: Encoder::new(encoder_a, encoder_b, EncodingType::K1X)
                .expect("Unable to create subsystem.drive encoder"),
        }
    }

    pub fn set(&mut self, speed: f64) {
        self.master.set(speed).expect("Unable to set pwm!");
        self.slave.set(speed).expect("Unable to set pwm!");
    }

    pub fn set_inverted(&mut self, inverted: bool) {
        self.master.set_inverted(inverted);
        self.slave.set_inverted(inverted);
    }

    pub fn rate(&self) -> f64 {
        self.encoder.rate().expect("Unable to read from encoder!")
    }

    /// Get my guess at the average motor voltage. I do not completely know what the base voltage
    /// is. Used in pwm testing.
    #[allow(dead_code)]
    pub fn voltage(&mut self) -> f64 {
        self.master.get().expect("Unable to read from subsystem.drive pwm") * 5.0
    }

    pub fn position(&self) -> i32 {
        self.encoder.get().expect("Unable to read from encoder")
    }
}
