use crate::subsystem::drive::drive_commands::{MoveCommand, MoveType};
use crate::subsystem::drive::drive_side::{DriveSide, PwmDriveSide, TalonDriveSide};
use crate::util::config::drive::WHEELBASE_WIDTH_FT;
use crate::util::config::drive_side::pwm::*;
use crate::util::config::drive_side::{LEFT_MASTER, LEFT_SLAVE, RIGHT_MASTER, RIGHT_SLAVE};
use crate::util::config::gear_shifter::*;
use crate::util::talon_factory::*;
use navx::AHRS;
use wpilib::encoder::{Encoder, EncodingType};
use wpilib::pneumatics::{Action, DoubleSolenoid};
use wpilib::pwm::PwmSpeedController;
use wpilib::spi::Port::MXP;

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

    pub fn execute_drive_command(&mut self, cmd: &MoveCommand) {
        match cmd.move_type() {
            MoveType::Velocity => {
                self.left_drive_side.set_velocity(cmd.left());
                self.right_drive_side.set_velocity(cmd.right());
            }
            MoveType::Percent => {
                self.left_drive_side.set_percent(cmd.left());
                self.right_drive_side.set_percent(cmd.right());
            }
        }
    }

    pub fn set_gear(&mut self, state: Action) {
        self.gear_shifter
            .set(state)
            .expect("Unable to set gear shifter!")
    }

    pub fn gear(&self) -> Action {
        self.gear_shifter
            .get()
            .expect("Unable to get gear shifter state!")
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
            gear_shifter: DoubleSolenoid::new(LOW_GEAR_CHANNEL, HIGH_GEAR_CHANNEL)
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
            gear_shifter: DoubleSolenoid::new(LOW_GEAR_CHANNEL, HIGH_GEAR_CHANNEL)
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
    pub fn voltage(&mut self) -> f64 {
        self.master.get().expect("Unable to read from subsystem.drive pwm") * 5.0
    }

    pub fn position(&self) -> i32 {
        self.encoder.get().expect("Unable to read from encoder")
    }
}
