use std::thread;

use bus::Bus;
use crossbeam_channel::Receiver;
use ctre::motor_control::config::*;
use ctre::motor_control::*;
use ctre::ErrorCode;
use navx::AHRS;
use wpilib::pneumatics::{Action, DoubleSolenoid};

use controls::units::*;

use crate::config::drive::*;

use super::Subsystem;

#[derive(Debug, PartialEq, Copy, Clone)]
pub struct Pose {
    pub x: Meter<f64>,
    pub y: Meter<f64>,
    pub heading: f64,
    pub velocity: MeterPerSecond<f64>,
    pub angular_velocity: Hertz<f64>,
    distance_accumulated: Meter<f64>,
}

#[derive(Debug, Copy, Clone)]
#[allow(dead_code)]
pub enum Instruction {
    GearShift(Action),
    Velocity(MeterPerSecond<f64>, MeterPerSecond<f64>),
    Percentage(f64, f64),
    FramePeriod(u8),
}

pub struct Drive {
    left_drive_side: DriveSide,
    right_drive_side: DriveSide,
    ahrs: AHRS,
    gear_shifter: DoubleSolenoid,
    receiver: Receiver<Instruction>,
    broadcaster: Bus<Pose>,
}

fn create_cim(id: i32) -> Result<TalonSRX, ErrorCode> {
    let mut talon: TalonSRX = TalonSRX::new(id);
    let mut config = TalonSRXConfig::default();
    config.base.voltageMeasurementFilter = 5;
    config.base.velocityMeasurementPeriod = VelocityMeasPeriod::Period_5Ms;
    config.forwardLimitSwitchSource = LimitSwitchSource::Deactivated;
    config.reverseLimitSwitchSource = LimitSwitchSource::Deactivated;
    config.peakCurrentLimit = CURRENT_LIMIT_THRESHOLD;
    config.peakCurrentDuration = CURRENT_LIMIT_DURATION_MS;
    config.continuousCurrentLimit = CURRENT_LIMIT;

    talon.config_all(&config, TALON_CONFIG_TIMEOUT_MS)?;

    talon.set_sensor_phase(false);
    talon.set_neutral_mode(NeutralMode::Brake);

    talon.config_selected_feedback_sensor(
        FeedbackDevice::CTRE_MagEncoder_Relative,
        0,
        TALON_CONFIG_TIMEOUT_MS,
    )?;
    talon.set_selected_sensor_position(0, 0, TALON_CONFIG_TIMEOUT_MS)?;
    talon.set_quadrature_position(0, TALON_CONFIG_TIMEOUT_MS)?;
    talon.enable_current_limit(true);

    Ok(talon)
}

impl Drive {
    /// Generates the next pose from the previous pose and current gyro data
    fn generate_pose(&self, previous: &Pose) -> Pose {
        let new_heading: f64 = self.ahrs.yaw().into();
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
            angular_velocity: (right_velocity - left_velocity) / DRIVE_BASE_WHEEL_WIDTH * 2.0,
            distance_accumulated: (left_distance + right_distance) / 2.0,
        }
    }

    pub fn new(broadcaster: Bus<Pose>, receiver: Receiver<Instruction>) -> Self {
        Drive {
            left_drive_side: DriveSide::new(
                create_cim(LEFT_MASTER).expect("Unable to create left master talon!"),
                create_cim(LEFT_SLAVE).expect("Unable to create left slave talon!"),
                true,
            )
            .expect("Unable to construct drive side!"),
            right_drive_side: DriveSide::new(
                create_cim(RIGHT_MASTER).expect("Unable to create right master talon!"),
                create_cim(RIGHT_SLAVE).expect("Unable to create right slave talon!"),
                false,
            )
            .expect("Unable to construct drive side!"),
            ahrs: AHRS::from_spi_minutiae(wpilib::spi::Port::MXP, 500_000, 60),
            gear_shifter: DoubleSolenoid::new(
                shifter::LOW_GEAR_CHANNEL,
                shifter::HIGH_GEAR_CHANNEL,
            )
            .expect("Unable to create gear shifter!"),
            receiver,
            broadcaster,
        }
    }
}

impl Subsystem for Drive {
    fn run(mut self) {
        let mut latest_pose = Pose {
            x: 0.0 * M,
            y: 0.0 * M,
            heading: 0.0,
            velocity: 0.0 * M / S,
            angular_velocity: 0.0 / S,
            distance_accumulated: 0.0 * M,
        };

        loop {
            thread::sleep(crate::config::SUBSYSTEM_SLEEP_TIME);

            for item in self.receiver.try_iter() {
                match item {
                    Instruction::GearShift(a) => self
                        .gear_shifter
                        .set(a)
                        .expect("Unable to set gear shifter!"),
                    Instruction::Percentage(left, right) => {
                        self.left_drive_side.set_percent(left);
                        self.right_drive_side.set_percent(right);
                    }
                    Instruction::Velocity(left, right) => {
                        self.left_drive_side.set_velocity(left);
                        self.right_drive_side.set_velocity(right);
                    }
                    Instruction::FramePeriod(n) => {
                        self.left_drive_side.config_frame_period(n);
                        self.right_drive_side.config_frame_period(n);
                    }
                }
            }

            latest_pose = self.generate_pose(&latest_pose);
            self.broadcaster.broadcast(latest_pose);
        }
    }
}

struct DriveSide {
    master: TalonSRX,
    #[allow(dead_code)]
    slave: TalonSRX,
}

impl DriveSide {
    fn new(master: TalonSRX, slave: TalonSRX, inverted: bool) -> Result<Self, ErrorCode> {
        let mut drive_side: DriveSide = DriveSide { master, slave };
        drive_side.slave.set(
            ControlMode::Follower,
            drive_side.master.get_device_id().into(),
            DemandType::Neutral,
            0.0,
        )?;
        drive_side
            .master
            .config_kf(LOW_GEAR_VEL_PID_IDX, 0.279_714_286, 10)?;
        drive_side.master.config_kp(0, 0.5, 10)?;
        drive_side.master.config_ki(0, 0.0, 10)?;
        drive_side.master.config_kd(0, 2.5, 10)?;

        drive_side.master.set_inverted(inverted);
        drive_side.slave.set_inverted(inverted);

        Ok(drive_side)
    }

    /// Set how fast to sample data from the encoders
    fn config_frame_period(&mut self, period: u8) {
        self.master
            .set_status_frame_period(StatusFrameEnhanced::Status_3_Quadrature, period, 0)
            .expect("Unable to configure the talons!");
    }

    fn set_percent(&mut self, percentage: f64) {
        self.master
            .set(
                ControlMode::PercentOutput,
                percentage,
                DemandType::Neutral,
                0.0,
            )
            .expect("Unable to communicate with the talons!");
    }

    fn set_velocity(&mut self, velocity: MeterPerSecond<f64>) {
        self.master
            .set(
                ControlMode::Velocity,
                0.005 * *(velocity / MPS),
                DemandType::Neutral,
                0.0,
            )
            .expect("Unable to communicate with the talons!");
    }

    fn position(&self) -> Meter<f64> {
        f64::from(self.master.get_selected_sensor_position(0).unwrap()) * M
    }

    fn velocity(&self) -> MeterPerSecond<f64> {
        f64::from(self.master.get_selected_sensor_velocity(0).unwrap()) * ENCODER_METERS_PER_TICK
    }
}
