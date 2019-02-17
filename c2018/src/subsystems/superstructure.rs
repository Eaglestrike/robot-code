use super::Subsystem;
use crossbeam_channel::Receiver;

mod hatch_hardware;
use hatch_hardware::HatchHardware;

mod channel;
mod elevator;
use elevator::Elevator;

#[derive(Debug, Clone)]
pub struct PeriodicOuts {
    pub intk_pct: f64,
    pub outk_pct: f64,
    pub intk_pnm: bool,
    pub elev_pos: controls::units::Meter<f64>,
}

impl Default for PeriodicOuts {
    fn default() -> Self {
        Self {
            intk_pct: 0.0,
            outk_pct: 0.0,
            intk_pnm: IntakeExt::Ext.into(),
            elev_pos: 0.0 * controls::units::M,
        }
    }
}

// Manual errors ikr
#[derive(Debug, Copy, Clone)]
pub enum HalCtreError {
    Hal(wpilib::HalError),
    Ctre(ctre::ErrorCode),
}
impl From<wpilib::HalError> for HalCtreError {
    fn from(err: wpilib::HalError) -> Self {
        HalCtreError::Hal(err)
    }
}
impl From<ctre::ErrorCode> for HalCtreError {
    fn from(err: ctre::ErrorCode) -> Self {
        HalCtreError::Ctre(err)
    }
}

// Utility
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum HatchPneumaticExt {
    Extended,
    Retracted,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum IntakeExt {
    Ext,
    Retr,
}

impl Into<bool> for IntakeExt {
    fn into(self) -> bool {
        match self {
            IntakeExt::Ext => true,
            IntakeExt::Retr => false,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct HatchState {
    pub extend: HatchPneumaticExt,
    pub outtake: HatchPneumaticExt,
}

// Interface for the controlling channel
mod interface {
    use super::goal::BallGoalHeight;
    use super::goal::HatchGoalHeight;
    use super::HatchPneumaticExt;
    pub enum UserElevatorHeights {
        Low,
        Med,
        High,
        Cargo,
    }

    impl UserElevatorHeights {
        pub fn into_hatch(self) -> HatchGoalHeight {
            match self {
                UserElevatorHeights::Cargo => HatchGoalHeight::Low,
                UserElevatorHeights::Low => HatchGoalHeight::Low,
                UserElevatorHeights::Med => HatchGoalHeight::Med,
                UserElevatorHeights::High => HatchGoalHeight::High,
            }
        }

        pub fn into_ball(self) -> BallGoalHeight {
            match self {
                UserElevatorHeights::Cargo => BallGoalHeight::Cargo,
                UserElevatorHeights::Low => BallGoalHeight::Low,
                UserElevatorHeights::Med => BallGoalHeight::Med,
                UserElevatorHeights::High => BallGoalHeight::High,
            }
        }
    }

    pub enum Instruction {
        SetElevatorHeight(UserElevatorHeights),
        Unjam(bool),
        BallOuttake,
        BallIntake,
        AbortIntake,
        HatchExtend(HatchPneumaticExt),
        HatchOuttake(HatchPneumaticExt),
    }

}
pub use interface::*;

// Data types for goal states
mod goal {
    use super::HatchState;
    use controls::units;
    #[derive(Debug, Clone, PartialEq, Eq)]
    pub enum GoalState {
        Ball(BallGoalHeight),
        Hatch(HatchGoalHeight, HatchState),
    }

    #[derive(Debug, Clone, PartialEq, Eq)]
    pub enum BallGoalState {
        Move(BallGoalHeight),
        Unjam,
    }

    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    pub enum BallGoalHeight {
        None,
        Cargo,
        Low,
        Med,
        High,
    }

    // TODO tune these heights
    impl Into<units::Meter<f64>> for BallGoalHeight {
        fn into(self) -> units::Meter<f64> {
            use BallGoalHeight::*;
            match self {
                None => 0. * units::M,
                Low => 0. * units::M,
                Cargo => 0. * units::M,
                Med => 0. * units::M,
                High => 0. * units::M,
            }
        }
    }

    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    pub enum HatchGoalHeight {
        Low,
        Med,
        High,
    }
    // TODO tune these heights
    impl Into<units::Meter<f64>> for HatchGoalHeight {
        fn into(self) -> units::Meter<f64> {
            use HatchGoalHeight::*;
            match self {
                Low => 0. * units::M,
                Med => 0. * units::M,
                High => 0. * units::M,
            }
        }
    }
}

mod unjam;

use ctre::motor_control::{ControlMode, DemandType, MotorController, TalonSRX};
use wpilib::{pneumatics::Solenoid, HalResult};
#[derive(Debug)]
pub struct Superstructure {
    goal: goal::GoalState,
    unjam: unjam::UnjamState,
    hatch_hardware: HatchHardware,
    channel: channel::Channel,
    elevator: Elevator,
    im: CachingTalon,
    is: CachingSolenoid,
    om: CachingTalon,
    receiver: Receiver<Instruction>,
}
use crate::config::superstructure as config;
impl Superstructure {
    pub fn new(recv: Receiver<Instruction>) -> HalResult<Self> {
        Ok(Self {
            goal: goal::GoalState::Hatch(
                goal::HatchGoalHeight::Low,
                hatch_hardware::CLOSED_HATCH_STATE,
            ),
            unjam: unjam::UnjamState::Disabled,
            hatch_hardware: hatch_hardware::HatchHardware::new()?,
            channel: channel::Channel::new()?,
            elevator: elevator::Elevator::new()?,
            im: CachingTalon::new(TalonSRX::new(config::CHANNEL_TALON)),
            is: CachingSolenoid::new(Solenoid::new(config::INTAKE_SOLENOID)?)?,
            om: CachingTalon::new(TalonSRX::new(config::OUTTAKE_TALON)),
            receiver: recv,
        })
    }
}

impl Superstructure {
    fn flush_outs(&mut self, out: &PeriodicOuts) -> Result<(), HalCtreError> {
        // TODO replace with individual handling
        // or consider using Result::and() to chain these as is
        self.im.pct(out.intk_pct)?;
        self.is.set(out.intk_pnm)?;
        self.om.pct(out.outk_pct)?;
        self.elevator.set_goal(out.elev_pos);
        Ok(())
    }
}

impl Subsystem for Superstructure {
    fn run(mut self) {
        use goal::*;
        loop {
            let mut outs = PeriodicOuts::default();
            for msg in self.receiver.try_iter() {
                use Instruction::*;
                match msg {
                    HatchExtend(ext) => match &mut self.goal {
                        GoalState::Hatch(ref _height, ref mut ext_state) => {
                            ext_state.extend = ext;
                        }
                        _ => (),
                    },
                    HatchOuttake(ext) => match &mut self.goal {
                        GoalState::Hatch(ref _height, ref mut ext_state) => {
                            ext_state.outtake = ext;
                        }
                        _ => (),
                    },
                    Unjam(x) => self.unjam.set_enabled(x),
                    BallIntake => self.goal = GoalState::Ball(BallGoalHeight::None),
                    SetElevatorHeight(wanted) => match self.goal {
                        GoalState::Hatch(ref mut height, _) => {
                            *height = wanted.into_hatch();
                        }
                        GoalState::Ball(ref mut height) => {
                            if self.channel.is_in_carriage() {
                                *height = wanted.into_ball();
                            }
                        }
                    },
                    BallOuttake => {
                        self.channel.try_init_outk();
                    }
                    AbortIntake => {
                        if self.channel.try_abort_intk() {
                            self.goal = goal::GoalState::Hatch(
                                goal::HatchGoalHeight::Low,
                                hatch_hardware::CLOSED_HATCH_STATE,
                            );
                        }
                    }
                }
            }
            // process desired state
            match self.goal.clone() {
                GoalState::Hatch(height, ext_state) => {
                    // TODO log
                    self.channel.process_sensors(false).unwrap();
                    self.elevator.set_goal(height.into());
                    // TODO log
                    self.hatch_hardware.set(ext_state.clone()).unwrap();
                }
                GoalState::Ball(goal_height) => {
                    // TODO log error
                    self.hatch_hardware.set_closed().unwrap();
                    self.channel.idempotent_start();
                    if self.channel.is_done() {
                        self.goal = GoalState::Hatch(
                            HatchGoalHeight::Low,
                            hatch_hardware::CLOSED_HATCH_STATE,
                        );
                        self.channel.reset();
                    } else {
                        self.elevator.set_goal(goal_height.into());
                        // TODO log the two possible failures here
                        self.channel
                            .process_sensors(self.elevator.is_holding().unwrap_or(false))
                            .unwrap();
                    }
                }
            }
            // TODO log
            self.elevator.iterate().unwrap();

            self.channel.write_outs(&mut outs);
            // Unjam gets to override everyone else
            self.unjam.process();
            self.unjam.write_outs(&mut outs);
            // TODO log
            self.flush_outs(&outs).unwrap();
        }
    }
}

#[derive(Debug)]
struct CachingTalon(TalonSRX, (ControlMode, f64, DemandType, f64));

impl CachingTalon {
    pub fn new(x: TalonSRX) -> Self {
        Self(
            x,
            (
                ControlMode::Disabled,
                std::f64::NAN,
                DemandType::Neutral,
                std::f64::NAN,
            ),
        )
    }

    pub fn pct(&mut self, pct: f64) -> ctre::Result<()> {
        self.set(ControlMode::PercentOutput, pct, DemandType::Neutral, 0.0)
    }

    pub fn set(&mut self, c: ControlMode, f: f64, d: DemandType, g: f64) -> ctre::Result<()> {
        use controls::approx::abs_diff_eq;
        if self.1 .0 == c
            && abs_diff_eq!(self.1 .1, f)
            && self.1 .2 == d
            && abs_diff_eq!(self.1 .3, g)
        {
            return Ok(());
        }
        self.1 .0 = c;
        self.1 .1 = f;
        self.1 .2 = d;
        self.1 .3 = g;
        self.0.set(c, f, d, g)
    }

    pub fn talon(&self) -> &TalonSRX {
        &self.0
    }

    pub fn talon_mut(&mut self) -> &mut TalonSRX {
        // set the cache to something that will force a change next time
        self.1 .1 = std::f64::NAN;
        self.1 .3 = std::f64::NAN;
        &mut self.0
    }
}

#[derive(Debug)]
struct CachingSolenoid(Solenoid, bool);

impl CachingSolenoid {
    pub fn new(s: Solenoid) -> HalResult<Self> {
        s.set(false)?;
        Ok(Self(s, false))
    }

    pub fn set(&mut self, b: bool) -> HalResult<()> {
        if b == self.1 {
            return Ok(());
        }
        self.1 = b;
        self.0.set(b)
    }
}
