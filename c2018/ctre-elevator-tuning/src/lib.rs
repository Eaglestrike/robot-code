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

// WARNING: Changing this constant requires alterations to many configs using enum variants
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

        // these need to run at 200hz to guarantee response time for velocity queries for the friction feed-forward
        mt.set_status_frame_period(StatusFrameEnhanced::Status_2_Feedback0, 5, 20);
        mt.set_status_frame_period(StatusFrameEnhanced::Status_10_MotionMagic, 5, 20);
        mt.set_status_frame_period(StatusFrameEnhanced::Status_3_Quadrature, 5, 20);

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
                // elevator hella stiff, esp stiction
                let friction;
                if self.is_holding().unwrap_or(true) {
                    friction = 0.0;
                } else {

                let target_vel = self.mt.get_active_trajectory_velocity()?;
                let vel = self.mt.get_selected_sensor_velocity(RECT_PROF_PID_IDX)?;
                // fast way to ensure stiction is accounted for properly and the error is tiny
                friction = calculate_friction(f64::from(vel) + f64::from(target_vel.signum()) * std::f64::EPSILON);
                }


                // TODO could need to account for the case where the profile has ended
                const KFRICTION: f64 = 0.1;
                self.mt.set(
                    ControlMode::MotionMagic,
                    self.goal.into(),
                    DemandType::ArbitraryFeedForward,
                    KFRICTION * friction
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

// Rembember to add a small epsilon of desired direction so static friction is properly handled.
#[allow(non_upper_case_globals)]
fn calculate_friction(v: f64) -> f64 {
/*  // mathworks model
    // https://www.mathworks.com/help/physmod/simscape/ref/translationalfriction.html
    // actual constants
    // const sqrt2e: f64 = f64::sqrt(2.0 * std::f64::consts::E); // unusable on stable rust
    const sqrt2e: f64 = 2.33164398159712416003230828209780156612396240234375;

    // tune-able constants
    const Fbrk: f64 = 0.0; // sum of static and couloumbic friction, overcome from rest
    const vbrk: f64 = 0.0; // velocity of peak stribek friction
    const Fc: f64 = 0.0; // constant coloubmic friction force
    const f: f64 = 0.0; // Viscous friction coefficient

    // derived constants
    // const vst: f64 = vbrk * f64::sqrt(2.0); // unusable on stable rust
    const vst: f64 = vbrk * 1.4142135623730951454746218587388284504413604736328125;
    const vcoul: f64 = vbrk / 10.0;

    sqrt2e * (Fbrk - Fc) * f64::exp(-((v/vst).powi(2))) * (v / vst) + Fc * f64::tanh(v / vcoul) + f*v
*/

    // more directly-represneted stribeck curve
    // http://www.mogi.bme.hu/TAMOP/robot_applications/ch07.html#ch-8.4.1.1 eqn 8.19
    // https://www.desmos.com/calculator/w8zzkxw4nz for visualization

    // tune-able consts
    const Fs: f64 = 0.0; // static friction
    const Fc: f64 = 0.0; // coulombic friction
    const Fv: f64 = 0.0; // coeficcient of viscous friction
    const vs: f64 = 0.0; // characteristic velocity of stribeck curve (curve tuning param)
                         // dictates how fast Fs falls off to Fc in the Stribeck curve approximation

    // Tuning procedure:
    // 0. Tune the entire rest of the motion profile as best you can
    // 1. Set vs to a start based on the velocity of the mechanism.
    // My first guess is 2/3 of the velocity at which point things really start to get easier.
    // Large values will more compensate for static friction at higher velocities.
    // Since our elevator has problems with bearings being pressed too hard (probably forcing boundary lubrication)
    // this will be high
    // 2. Measure the motor command to overcome static friction. Set Fs to 1.0 and KFriction to the motor command.
    // If you like, you could remove KFriction entirely, its just an extra gain.
    // 3. Tweak vs until actual velocity no longer lags target velocity during initial acceleration
    // 4. Tweak Fc to further reduce velocity lag at V_cruise
    // You may want to reduce Kv during this process, as some of the gain from Fc was probably lumped in there to start with if you tuned right
    // 5. Tweak Fv. This is very close to just another Kv, but it runs on the actual velocity, not the desired velocity.
    // This may be able to make a noticeable difference. If you increase Kv from zero, decrease Ka and Kv, as again these two got lumped in there.
    // 6. Re-tune Kv

    // Alternative (preferred?) tuning procudure:
    // Run the mechanism hooked up to a joystick with the friction compensator as the only feedforward. No feedback.
    // Tune with the above procedure.
    // The tuning criteria should be:
    // 1. Change in output velocity should be responsive and proportional to a change in motor command
    // 2. Relatively quick, but not particularly fast, velocity settling time
    // 3. The above is true across all velocity ranges
    // Tune any closed loop system with the friction compensator on.

    // derived consts
    const rvs: f64 = 1.0 / vs; // inverse to save divide cycles

    v.signum()*(Fc + ((Fs - Fc)/(1.0 + v*rvs*v*rvs)) + Fv*v)
}

/*const*/ fn meters_per_second_to_ticks_per_100ms(v: si::MeterPerSecond<f64>) -> f64 {
    *(v / METERS_PER_TICK * (0.1 * si::S /* / 1 100ms interval*/ ))
}
