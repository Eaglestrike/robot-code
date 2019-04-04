use std::thread;

use bus::Bus;
use crossbeam_channel::Receiver;
use ctre::motor_control::config::*;
use ctre::motor_control::*;
use debug_stub_derive::DebugStub;
use lazy_static::lazy_static;
//use navx::AHRS;
use wpilib::pneumatics::Solenoid;

use controls::const_unit;
use controls::units::*;

use crate::config::drive::*;

use super::Subsystem;

#[derive(Debug, Copy, Clone)]
pub enum Gear {
    High,
    Low,
}

impl From<Gear> for bool {
    fn from(gear: Gear) -> Self {
        match gear {
            Gear::High => shifter::HIGH_GEAR,
            Gear::Low => !shifter::HIGH_GEAR,
        }
    }
}

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
pub enum Instruction {
    GearShift(Gear),
    Percentage(f64, f64),
}

#[derive(DebugStub)]
pub struct Drive {
    l_mstr: TalonSRX,
    r_mstr: TalonSRX,
    _l_slave: TalonSRX,
    _r_slave: TalonSRX,
    // #[debug_stub = "Opaque(AHRS)"]
    // ahrs: AHRS,
    gear_shifter: Solenoid,
    receiver: Receiver<Instruction>,
    #[debug_stub = "Opaque(Bus<Pose>)"]
    broadcaster: Bus<Pose>,
}

lazy_static! {
    static ref DEFAULT_CONFIG: TalonSRXConfig = TalonSRXConfig {
        base: BaseMotorConfig {
            voltageMeasurementFilter: 5,
            velocityMeasurementPeriod: VelocityMeasPeriod::Period_5Ms,
            ..Default::default()
        },
        forwardLimitSwitchSource: LimitSwitchSource::Deactivated,
        reverseLimitSwitchSource: LimitSwitchSource::Deactivated,
        peakCurrentLimit: CURRENT_LIMIT_THRESHOLD,
        peakCurrentDuration: CURRENT_LIMIT_DURATION_MS,
        continuousCurrentLimit: CURRENT_LIMIT,
        ..Default::default()
    };
}
trait TypedQuadrature {
    fn pos(&self) -> ctre::Result<Meter<f64>>;
    fn vel(&self) -> ctre::Result<MeterPerSecond<f64>>;
}

impl TypedQuadrature for TalonSRX {
    fn pos(&self) -> ctre::Result<Meter<f64>> {
        self.get_quadrature_position()
            .and_then(|ticks| Ok(f64::from(ticks) * crate::config::drive::ENCODER_METERS_PER_TICK))
    }

    fn vel(&self) -> ctre::Result<MeterPerSecond<f64>> {
        self.get_quadrature_velocity().and_then(|ticks| {
            Ok(f64::from(ticks) * crate::config::drive::ENCODER_METERS_PER_TICK / S)
        })
    }
}

impl Drive {
    /// Generates the next pose from the previous pose and current gyro data
    fn generate_pose(&self, previous: &Pose) -> Pose {
        // let new_heading: f64 = self.ahrs.yaw().into();
        let new_heading = 0.0;
        let angle = (new_heading + previous.heading) / 2.0;

        //TODO log errors handling here
        let left_distance = self.l_mstr.pos().unwrap_or(const_unit!(0.));
        let right_distance = self.r_mstr.pos().unwrap_or(const_unit!(0.));

        let left_velocity = self.l_mstr.vel().unwrap_or(const_unit!(0.));
        let right_velocity = self.l_mstr.vel().unwrap_or(const_unit!(0.));

        let distance = (left_distance + right_distance) / 2.0 - previous.distance_accumulated;

        Pose {
            x: previous.x + distance * angle.cos(),
            y: previous.y + distance * angle.sin(),
            heading: new_heading,
            velocity: (left_velocity + right_velocity) / 2.0,
            angular_velocity: (right_velocity - left_velocity) / DRIVE_BASE_WHEEL_WIDTH,
            distance_accumulated: (left_distance + right_distance) / 2.0,
        }
    }

    pub fn new(broadcaster: Bus<Pose>, receiver: Receiver<Instruction>) -> Self {
        // TODO log errors
        let mut l_mstr = TalonSRX::new(LEFT_MASTER);
        let mut l_slave = TalonSRX::new(LEFT_SLAVE);
        l_mstr.config_all(&DEFAULT_CONFIG, TALON_CFG_TO_MS).ok();
        // .expect("Unable to configure left_master side!")
        l_slave.config_all(&DEFAULT_CONFIG, TALON_CFG_TO_MS).ok();
        // .expect("Unable to configure left_slave side!");
        l_slave
            .follow(&l_mstr, FollowerType::PercentOutput)
            .unwrap();
        l_mstr.set_inverted(true);
        l_slave.set_inverted(true);

        let mut r_mstr = TalonSRX::new(RIGHT_MASTER);
        let mut r_slave = TalonSRX::new(RIGHT_SLAVE);
        r_mstr.config_all(&DEFAULT_CONFIG, TALON_CFG_TO_MS).ok();
        // .expect("Unable to configure right_master side!");
        r_slave.config_all(&DEFAULT_CONFIG, TALON_CFG_TO_MS).ok();
        // .expect("Unable to configure right_slave side!");
        r_slave
            .follow(&r_mstr, FollowerType::PercentOutput)
            .unwrap();

        Self {
            l_mstr,
            r_mstr,
            _l_slave: l_slave,
            _r_slave: r_slave,
            // ahrs: AHRS::from_spi_minutiae(wpilib::spi::Port::MXP, 500_000, 60),
            // TODO log errors
            gear_shifter: Solenoid::new(shifter::SOLENOID_CHANNEL)
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
                    Instruction::GearShift(g) => self
                        .gear_shifter
                        .set(g.into())
                        // TODO log
                        .expect("Unable to set gear shifter!"),
                    Instruction::Percentage(lpct, rpct) => {
                        self.l_mstr
                            .set(ControlMode::PercentOutput, lpct, DemandType::Neutral, 0.0)
                            .unwrap();
                        self.r_mstr
                            .set(ControlMode::PercentOutput, rpct, DemandType::Neutral, 0.0)
                            .unwrap();
                    }
                }
            }

            latest_pose = self.generate_pose(&latest_pose);
            self.broadcaster.broadcast(latest_pose);
        }
    }
}
