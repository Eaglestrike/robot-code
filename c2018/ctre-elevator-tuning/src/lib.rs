pub mod config {
    // todo IDS
    pub const MASTER_TALON: i32 = 13;
    pub const SLAVE_TALON1: i32 = 4;
    pub const SLAVE_TALON2: i32 = 6;
    pub const LIMIT_SWITCH: i32 = 9;
}
use controls::const_unit;
use controls::units as si;
use controls::util::clamp;
use ctre::motor_control::config::*;
use ctre::motor_control::*;
use wpilib::{dio::DigitalInput, HalResult};

#[derive(Copy, Clone, Debug)]
pub enum LoopState {
    Unitialized,
    Zeroing,
    Running,
}

#[derive(Debug)]
pub struct Elevator {
    mt: TalonSRX,
    s1: TalonSRX,
    s2: TalonSRX,
    limit: DigitalInput,
    state: LoopState,
    goal: i32,         // encoder ticks
    last_sent_sp: i32, // encoder ticks
}

const RECT_PROF_PID_IDX: i32 = 0;
const ZEROING_COMMAND: f64 = -0.3;
// TODO tune these
const GRAVITY_KF: f64 = 0.05;
const COMPLETION_THRESHOLD: si::Meter<f64> = const_unit!(0.01);
const COMPLETION_THRESHOLD_TICKS: i32 =
    (COMPLETION_THRESHOLD.value_unsafe / METERS_PER_TICK.value_unsafe) as i32; // value_unsafe because no const_fn yet.

use std::f64::consts::PI;
pub const METERS_PER_TICK: si::Meter<f64> = const_unit!(1.982 /*in*/ * 0.0254 * PI / 4096.0);

impl Elevator {
    pub const ZEROING_SPEED: si::MeterPerSecond<f64> = const_unit!(0.04);
    pub const MAX_HEIGHT: si::Meter<f64> = const_unit!(2.0);
    pub const MIN_HEIGHT: si::Meter<f64> = const_unit!(-0.02);
    pub const MAX_HEIGHT_TICKS: i32 = 38500;
    pub const MIN_HEIGHT_TICKS: i32 = -1;
    // pub const TARGET_KF: f64 = 0.00004;
    pub const TARGET_KF: f64 = 0.0;
    pub const DT: si::Second<f64> = const_unit!(1. / 200.);
    pub const KP: si::VoltPerMeter<f64> = const_unit!(300.0);
    pub const KD: si::VoltSecondPerMeter<f64> = const_unit!(30.);
    pub const KF: si::Volt<f64> = const_unit!(1.5);

    pub fn new() -> HalResult<Self> {
        // TODO config the talons
        let mut mt = TalonSRX::new(config::MASTER_TALON);
        mt.config_all(
            &TalonSRXConfig {
                primaryPID: TalonSRXPIDSetConfiguration {
                    selectedFeedbackSensor: FeedbackDevice::CTRE_MagEncoder_Relative,
                    ..Default::default()
                },
                base: BaseMotorConfig {
                    forwardSoftLimitThreshold: Self::MAX_HEIGHT_TICKS,
                    forwardSoftLimitEnable: true,
                    reverseSoftLimitThreshold: Self::MIN_HEIGHT_TICKS,
                    reverseSoftLimitEnable: true,
                    voltageCompSaturation: 12.0,
                    slot_0: SlotConfiguration {
                        kP: 0.15,
                        kI: 0.0,
                        kD: 4.0,
                        kF: 0.1,
                        integralZone: 700,
                        allowableClosedloopError: 0,
                        maxIntegralAccumulator: 99999999.0,
                        closedLoopPeakOutput: 1.0,
                        closedLoopPeriod: 1,
                    },
                    motionCruiseVelocity: 12500,
                    motionAcceleration: 10000,
                    motionProfileTrajectoryPeriod: 0,
                    closedloopRamp: 0.1,
                    openloopRamp: 0.1,
                    ..Default::default()
                },
                continuousCurrentLimit: 20,
                peakCurrentLimit: 35,
                peakCurrentDuration: 200,
                ..Default::default()
            },
            100,
        )
        .expect("CONFIG ALL FAILED");
        mt.enable_current_limit(true);
        mt.select_profile_slot(0, 0);
        mt.override_limit_switches_enable(true);
        mt.override_soft_limits_enable(false); // enabled after zeroing
        mt.enable_voltage_compensation(true);
        mt.set_status_frame_period(StatusFrameEnhanced::Status_2_Feedback0, 10, 20);
        mt.set_status_frame_period(StatusFrameEnhanced::Status_10_MotionMagic, 10, 20);

        mt.set_inverted(false);
        mt.set_sensor_phase(true);
        mt.set_neutral_mode(NeutralMode::Brake);

        let mut s1 = TalonSRX::new(config::SLAVE_TALON1);
        s1.follow(&mt, FollowerType::PercentOutput)
            .expect("COULD NOT FOLLOW");
        let mut s2 = TalonSRX::new(config::SLAVE_TALON2);
        s2.follow(&mt, FollowerType::PercentOutput)
            .expect("COULD NOT FOLLOW");

        Ok(Self {
            mt,
            s1,
            s2,
            limit: DigitalInput::new(config::LIMIT_SWITCH)?,
            state: LoopState::Unitialized,
            goal: std::i32::MIN, // ticks
            last_sent_sp: std::i32::MIN,
        })
    }
}

impl Elevator {
    pub fn iterate(&mut self) -> ctre::Result<()> {
        match self.state {
            LoopState::Unitialized => {
                // TODO handle
                self.state = LoopState::Zeroing;
                return self.iterate();
            }
            LoopState::Zeroing => {
                match self.limit.get() {
                    Err(_) => {
                        // TODO log
                        self.mt
                            .set(ControlMode::PercentOutput, 0.0, DemandType::Neutral, 0.0)
                    }
                    // limit is normally closed
                    Ok(true) => self.mt.set(
                        ControlMode::PercentOutput,
                        ZEROING_COMMAND,
                        DemandType::Neutral,
                        0.0,
                    ),
                    Ok(false) => {
                        // TODO log
                        self.mt
                            .set_selected_sensor_position(0, RECT_PROF_PID_IDX, 10)
                            .expect("SELECTED SENSOR POSITION");
                        // .ok_print();
                        self.state = LoopState::Running;
                        self.mt.override_soft_limits_enable(true);
                        self.set_goal(0.0 * si::M);
                        return self.iterate();
                    }
                }
            }
            LoopState::Running => {
                self.mt.set(
                    ControlMode::MotionMagic,
                    self.goal.into(),
                    DemandType::ArbitraryFeedForward,
                    Self::TARGET_KF
                        * (self.goal - ((Self::MAX_HEIGHT_TICKS - Self::MIN_HEIGHT_TICKS) / 2))
                            as f64,
                    // DemandType::Neutral,
                    // 0.0,
                )?;
                self.last_sent_sp = self.goal;
                // dbg!(self.mt.get_selected_sensor_position(0));
                // dbg!(self.goal);
                Ok(())
            }
        }
    }

    pub fn set_goal(&mut self, sp: si::Meter<f64>) {
        self.goal = clamp(
            *(sp / METERS_PER_TICK) as i32,
            Self::MIN_HEIGHT_TICKS,
            Self::MAX_HEIGHT_TICKS,
        );
        // dbg!(self.goal);
    }

    pub fn is_holding(&self) -> ctre::Result<bool> {
        Ok(self.last_sent_sp == self.goal
            && (self.mt.get_selected_sensor_position(RECT_PROF_PID_IDX)? - self.last_sent_sp).abs()
                < COMPLETION_THRESHOLD_TICKS)
    }

    #[allow(dead_code)]
    pub fn is_at_height(&self, pos: si::Meter<f64>) -> ctre::Result<bool> {
        let ticks: i32 = *(pos / METERS_PER_TICK) as i32;
        Ok(
            (self.mt.get_selected_sensor_position(RECT_PROF_PID_IDX)? - ticks).abs()
                < COMPLETION_THRESHOLD_TICKS,
        )
    }

    #[allow(dead_code)]
    pub fn state(&self) -> LoopState {
        self.state
    }
}
