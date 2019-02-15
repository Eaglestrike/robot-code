use crate::config::superstructure::elevator as config;
use controls::const_unit;
use controls::units as si;
use controls::util::clamp;
use ctre::motor_control::{ControlMode, DemandType, MotorController, TalonSRX};
use wpilib::{dio::DigitalInput, HalResult};

#[derive(Copy, Clone, Debug)]
enum LoopState {
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
    goal: i32, // encoder ticks
    last_sent_sp: i32, // encoder ticks
}

const RECT_PROF_PID_IDX: i32 = 1;
const ZEROING_COMMAND: f64 = -0.02;
// TODO tune these
const GRAVITY_KF: f64 = 0.02;
const COMPLETION_THRESHOLD: si::Meter<f64> = const_unit!(0.01);
const COMPLETION_THRESHOLD_TICKS: i32 =
    (COMPLETION_THRESHOLD.value_unsafe / METERS_PER_TICK.value_unsafe) as i32; // value_unsafe because no const_fn yet.

use std::f64::consts::PI;
const METERS_PER_TICK: si::Meter<f64> = const_unit!(1.982 /*in*/ * 0.0254 * PI / 4096.0);

impl Elevator {
    const ZEROING_SPEED: si::MeterPerSecond<f64> = const_unit!(0.04);
    const MAX_HEIGHT: si::Meter<f64> = const_unit!(2.1336);
    const MIN_HEIGHT: si::Meter<f64> = const_unit!(-0.02);
    const DT: si::Second<f64> = const_unit!(1. / 200.);
    const KP: si::VoltPerMeter<f64> = const_unit!(300.0);
    const KD: si::VoltSecondPerMeter<f64> = const_unit!(30.);
    const KF: si::Volt<f64> = const_unit!(1.5);

    pub fn new() -> HalResult<Self> {
        // TODO config the talons
        Ok(Self {
            mt: TalonSRX::new(config::MASTER_TALON),
            s1: TalonSRX::new(config::SLAVE_TALON1),
            s2: TalonSRX::new(config::SLAVE_TALON2),
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
                return self.iterate();
            }
            LoopState::Zeroing => {
                match self.limit.get() {
                    Err(_) => {
                        // TODO log
                        self.mt
                            .set(ControlMode::PercentOutput, 0.0, DemandType::Neutral, 0.0)
                    }
                    Ok(false) => self.mt.set(
                        ControlMode::PercentOutput,
                        ZEROING_COMMAND,
                        DemandType::Neutral,
                        0.0,
                    ),
                    Ok(true) => {
                        // TODO log
                        self.mt
                            .set_selected_sensor_position(0, RECT_PROF_PID_IDX, 10)
                            .unwrap();
                        // TOOD config soft limits
                        self.state = LoopState::Running;
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
                    GRAVITY_KF,
                )?;
                self.last_sent_sp = self.goal;
                Ok(())
            }
        }
    }

    pub fn set_goal(&mut self, sp: si::Meter<f64>) {
        let sp = clamp(sp, Self::MIN_HEIGHT, Self::MAX_HEIGHT);
        self.goal = *(sp / METERS_PER_TICK) as i32;;
    }

    pub fn is_holding(&self) -> ctre::Result<bool> {
        Ok(
            self.last_sent_sp == self.goal && (self.mt.get_selected_sensor_position(RECT_PROF_PID_IDX)? - self.last_sent_sp).abs()
                < COMPLETION_THRESHOLD_TICKS,
        )
    }

    pub fn is_at_height(&self, pos: si::Meter<f64>) -> ctre::Result<bool> {
        let ticks: i32 = *(pos / METERS_PER_TICK) as i32;
        Ok(
            (self.mt.get_selected_sensor_position(RECT_PROF_PID_IDX)? - ticks).abs()
                < COMPLETION_THRESHOLD_TICKS,
        )
    }

    pub fn state(&self) -> LoopState {
        self.state
    }
}
