use self::drive_side::{DriveSide, PwmDriveSide, TalonDriveSide};
use crate::subsystem::Subsystem;
use crate::util::config::drive::{pwm::*, *};
use crate::util::config::{BUS_SIZE, TICK_RATE};
use crate::util::talon_factory::*;

use navx::AHRS;

use wpilib::encoder::{Encoder, EncodingType};
use wpilib::pneumatics::{Action, DoubleSolenoid};
use wpilib::pwm::PwmSpeedController;
use wpilib::spi::Port::MXP;

use bus::{Bus, BusReader};
use crossbeam_channel::{unbounded, Receiver, Sender};
use std::thread;

mod drive_side;

#[derive(Debug, PartialEq, Default, Copy, Clone)]
pub struct Pose {
    pub x: f64,
    pub y: f64,
    pub heading: f64,
    pub velocity: f64,
    pub angular_velocity: f64,
    distance_accumulated: f64,
}

#[derive(Debug, Copy, Clone)]
pub enum Instruction {
    GearShift(Action),
    Velocity(f64, f64),
    Percentage(f64, f64),
}
pub struct Drive<T: DriveSide> {
    left_drive_side: T,
    right_drive_side: T,
    ahrs: AHRS,
    gear_shifter: DoubleSolenoid,
    receiver: Receiver<Instruction>,
    pose_broadcaster: Bus<Pose>,
}

impl<T: DriveSide> Drive<T> {
    /// Generates the next pose from the previous pose and current gyro data
    fn generate_pose(&self, previous: &Pose) -> Pose {
        let new_heading = self.ahrs.yaw() as f64;
        let angle = (new_heading + previous.heading) / 2.0;

        let left_distance = self.left_drive_side.position();
        let right_distance = self.right_drive_side.position();

        let left_velocity = self.left_drive_side.velocity();
        let right_velocity = self.right_drive_side.velocity();

        let distance = (left_distance + right_distance) / 2.0 - previous.distance_accumulated;

        Pose {
            x: previous.x + distance * angle.cos(),
            y: previous.y + distance * angle.sin(),
            heading: new_heading,
            velocity: (left_velocity + right_velocity) / 2.0,
            angular_velocity: right_velocity - left_velocity / DRIVE_BASE_WHEEL_WIDTH * 2.0,
            distance_accumulated: (left_distance + right_distance) / 2.0,
        }
    }
}

impl Drive<TalonDriveSide> {
    pub fn new() -> (Self, Sender<Instruction>) {
        let (sender, receiver) = unbounded();

        let drive = Drive {
            left_drive_side: TalonDriveSide::new(
                motor_type::CtreCim::create(LEFT_MASTER),
                motor_type::CtreCim::create(LEFT_SLAVE),
            ),
            right_drive_side: TalonDriveSide::new(
                motor_type::CtreCim::create(RIGHT_MASTER),
                motor_type::CtreCim::create(RIGHT_SLAVE),
            ),
            ahrs: AHRS::from_spi_minutiae(MXP, 500_000, 60),
            gear_shifter: DoubleSolenoid::new(
                shifter::LOW_GEAR_CHANNEL,
                shifter::HIGH_GEAR_CHANNEL,
            )
            .expect("Unable to create gear shifter!"),
            receiver,
            pose_broadcaster: Bus::new(BUS_SIZE),
        };

        (drive, sender)
    }
}

impl Drive<drive_side::PwmDriveSide> {
    pub fn new() -> (Self, Sender<Instruction>) {
        let (sender, receiver) = unbounded();

        let drive = Drive {
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
            gear_shifter: DoubleSolenoid::new(
                shifter::LOW_GEAR_CHANNEL,
                shifter::HIGH_GEAR_CHANNEL,
            )
            .expect("Unable to create gear shifter!"),
            receiver,
            pose_broadcaster: Bus::new(BUS_SIZE),
        };

        (drive, sender)
    }
}

impl<T: DriveSide> Subsystem<Pose> for Drive<T> {
    fn run(&mut self) {
        let mut latest_pose = Pose::default();

        loop {
            thread::sleep(TICK_RATE);

            for item in self.receiver.try_iter() {
                match item {
                    Instruction::GearShift(a) => self
                        .gear_shifter
                        .set(a)
                        .expect("Unable to set gear shifter!"),
                    Instruction::Percentage(left, right) => {
                        self.left_drive_side.set_percent(left.min(1.0).max(-1.0));
                        self.right_drive_side.set_percent(right.min(1.0).max(-1.0));
                    }
                    Instruction::Velocity(left, right) => {
                        self.left_drive_side
                            .set_velocity(left.min(MAX_VELOCITY).max(-MAX_VELOCITY));
                        self.right_drive_side
                            .set_velocity(right.min(MAX_VELOCITY).max(-MAX_VELOCITY));
                    }
                }
            }

            latest_pose = self.generate_pose(&latest_pose);
            send_bus!(self.pose_broadcaster, latest_pose.clone());
        }
    }

    fn create_receiver(&mut self) -> BusReader<Pose> {
        self.pose_broadcaster.add_rx()
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
            master: PwmSpeedController::new_talon(motor_a)
                .expect("Unable to create subsystem.drive pwm"),
            slave: PwmSpeedController::new_talon(motor_b)
                .expect("Unable to create subsystem.drive pwm"),
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
        self.master
            .get()
            .expect("Unable to read from subsystem.drive pwm")
            * 5.0
    }

    pub fn position(&self) -> i32 {
        self.encoder.get().expect("Unable to read from encoder")
    }
}
