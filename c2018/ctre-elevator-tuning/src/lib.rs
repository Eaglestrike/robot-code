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
    stage_track: StageTracker,
}

const RECT_PROF_PID_IDX: i32 = 0;
const ZEROING_COMMAND: f64 = -0.3;
// TODO tune these
const COMPLETION_THRESHOLD: si::Meter<f64> = const_unit!(0.03);
const COMPLETION_THRESHOLD_TICKS: i32 =
    (COMPLETION_THRESHOLD.value_unsafe / METERS_PER_TICK.value_unsafe) as i32; // value_unsafe because no const_fn yet.

use std::f64::consts::PI;
pub const METERS_PER_TICK: si::Meter<f64> = const_unit!(1.982 /*in*/ * 0.0254 * PI / 4096.0);

const STAGE_ONE_SLOT_IDX: i32 = 0;
const STAGE_TWO_SLOT_IDX: i32 = 1;

const GRAVITY_KF: f64 = 0.06;
const STAGE_ONE_FRICTION_FF: f64 = 0.05;
const STAGE_TWO_FRICTION_FF: f64 = 0.07;

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
                    // Stage one slot
                    slot_0: SlotConfiguration {
                        kP: 0.18,
                        kI: 0.0,
                        kD: 4.0,
                        kF: 0.04,
                        integralZone: 0,
                        allowableClosedloopError: 0,
                        maxIntegralAccumulator: 0.0,
                        closedLoopPeakOutput: 1.0,
                        closedLoopPeriod: 1,
                    },
                    // stage two slot (more rigid)
                    slot_1: SlotConfiguration {
                        kP: 0.23,
                        kI: 0.0,
                        kD: 2.0,
                        kF: 0.04,
                        integralZone: 0,
                        allowableClosedloopError: 0,
                        maxIntegralAccumulator: 0.0,
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
            3000,
        )
        .expect("CONFIG ALL FAILED");
        mt.enable_current_limit(true);
        mt.select_profile_slot(STAGE_ONE_SLOT_IDX, RECT_PROF_PID_IDX);
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
            stage_track: StageTracker::zeroed(),
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
                            .set_selected_sensor_position(0, RECT_PROF_PID_IDX, 1000)
                            .expect("SELECTED SENSOR POSITION");
                        // .ok_print();
                        self.stage_track = StageTracker::zeroed();
                        self.state = LoopState::Running;
                        self.mt.override_soft_limits_enable(true);
                        self.set_goal(0.0 * si::M);
                        // we need to set the mode so get_closed_loop_target works in the next cycle
                        self.mt.set(
                            ControlMode::MotionMagic,
                            self.goal.into(),
                            DemandType::Neutral,
                            0.0,
                        )?;
                        return self.iterate();
                    }
                }
            }
            LoopState::Running => {
                let current_pos = self.mt.get_selected_sensor_position(RECT_PROF_PID_IDX)?;
                self.stage_track.update(current_pos);
                let err = self.mt.get_closed_loop_target(RECT_PROF_PID_IDX)? - current_pos;
                let moving_stage = self.stage_track.moving_stage(err);
                // dbg!(moving_stage);
                self.mt
                    .select_profile_slot(moving_stage.slot_idx(), RECT_PROF_PID_IDX)?;
                self.mt.set(
                    ControlMode::MotionMagic,
                    self.goal.into(),
                    DemandType::ArbitraryFeedForward,
                    GRAVITY_KF + f64::from(err.signum()) * moving_stage.ff(),
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

use controls;

#[derive(Debug, Copy, Clone)]
pub enum Stage {
    One,
    Two,
}

impl Stage {
    pub fn slot_idx(self) -> i32 {
        match self {
            Stage::One => STAGE_ONE_SLOT_IDX,
            Stage::Two => STAGE_TWO_SLOT_IDX,
        }
    }

    pub fn ff(self) -> f64 {
        match self {
            Stage::One => STAGE_ONE_FRICTION_FF,
            Stage::Two => STAGE_TWO_FRICTION_FF,
        }
    }
}

#[derive(Debug, Clone)]
pub struct StageTracker {
    stage_two_pos: i32,
    old_pos_ticks: i32,
}

impl StageTracker {
    // TODO tune
    /// Read encoder value when both stages have hit their bottom hardstops
    const LOWEST_POS_TICKS: i32 = 0;
    /// Read encoder value when the second stage rests on the first, and the first stage contacts the top hardstop
    const HIGHEST_POS_TICKS: i32 = 23000;

    pub fn zeroed() -> Self {
        Self {
            stage_two_pos: 0,
            old_pos_ticks: 0,
        }
    }

    pub fn moving_stage(&self, direction: i32) -> Stage {
        if self.at_top_limit() && direction > 0 {
            Stage::Two
        } else if self.at_bot_limit() && direction < 0 {
            Stage::Two
        } else {
            Stage::One
        }
    }

    fn at_top_limit(&self) -> bool {
        self.stage_two_pos >= Self::HIGHEST_POS_TICKS
    }
    fn at_bot_limit(&self) -> bool {
        self.stage_two_pos <= Self::LOWEST_POS_TICKS
    }

    pub fn stage_two_pos(&self) -> i32 {
        self.stage_two_pos
    }

    pub fn update(&mut self, new_pos: i32) {
        let delta = new_pos - self.old_pos_ticks;
        self.stage_two_pos += delta;
        self.stage_two_pos = controls::util::clamp(
            self.stage_two_pos,
            Self::LOWEST_POS_TICKS,
            Self::HIGHEST_POS_TICKS,
        );
        self.old_pos_ticks = new_pos;
    }
}
