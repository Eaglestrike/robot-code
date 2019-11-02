use super::HatchPneumaticExt;
use super::HatchState;
use crate::config::superstructure::hatch as config;
use wpilib::pneumatics::Solenoid;
use wpilib::HalResult;

#[derive(Debug)]
pub struct HatchHardware {
    extend: Solenoid,
    outtake: Solenoid,
    state: HatchState,
}

pub const CLOSED_HATCH_STATE: HatchState = HatchState {
    extend: HatchPneumaticExt::Retracted,
    outtake: HatchPneumaticExt::Retracted,
};

impl HatchHardware {
    pub fn new() -> HalResult<Self> {
        Ok(Self {
            extend: Solenoid::new(config::EXTEND_PNEUMATICS_ID)?,
            outtake: Solenoid::new(config::OUTTAKE_PNEUMATICS_ID)?,
            state: HatchState {
                extend: HatchPneumaticExt::Retracted,
                outtake: HatchPneumaticExt::Retracted,
            },
        })
    }

    pub fn set(&mut self, new: HatchState) -> HalResult<()> {
        if self.state.extend != new.extend {
            self.extend.set(match new.extend {
                HatchPneumaticExt::Extended => true,
                HatchPneumaticExt::Retracted => false,
            })?;
        }
        if self.state.outtake != new.outtake {
            self.outtake.set(match new.outtake {
                HatchPneumaticExt::Extended => true,
                HatchPneumaticExt::Retracted => false,
            })?;
        }
        self.state = new;
        Ok(())
    }

    pub fn set_closed(&mut self) -> HalResult<()> {
        self.set(HatchState {
            extend: HatchPneumaticExt::Retracted,
            outtake: HatchPneumaticExt::Retracted,
        })
    }
}
