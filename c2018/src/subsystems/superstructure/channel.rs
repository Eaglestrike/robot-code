use super::HalCtreError;
use super::IntakeExt;
use super::PeriodicOuts;
use wpilib::{dio::DigitalInput, HalResult};

// TODO tune
const CHAN_INTAKE_COMMAND: f64 = 0.8;
const CHAN_CONVEY_COMMAND: f64 = 0.6;
const CHAN_TRANSFER_COMMAND: f64 = 1.0;
const OUTK_INTK_COMMAND: f64 = 0.8;
const OUTK_OUTK_COMMAND: f64 = 1.0;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum BallProgress {
    None,
    Intaking,
    Inside,
    Queued,
    CarriageVolatile,
    CarriageSecure,
    Outtaking(u16),
    Done,
}

#[derive(Debug)]
pub struct Channel {
    state: BallProgress,
    gates: (DigitalInput, DigitalInput, DigitalInput),
}

const OUTK_TIME_TICKS: u16 = 200;

use crate::config::superstructure::{GATE1, GATE2, GATE3};
impl Channel {
    pub fn new() -> HalResult<Self> {
        Ok(Self {
            state: BallProgress::None,
            gates: (
                DigitalInput::new(GATE1)?,
                DigitalInput::new(GATE2)?,
                DigitalInput::new(GATE3)?,
            ),
        })
    }

    pub fn try_abort_intk(&mut self) -> bool {
        use BallProgress::*;
        return match self.state {
            Intaking => {
                self.state = None;
                true
            }
            _ => false,
        };
    }

    pub fn try_init_outk(&mut self) -> bool {
        use BallProgress::*;
        return match self.state {
            CarriageSecure => {
                self.state = Outtaking(OUTK_TIME_TICKS);
                true
            }
            _ => false,
        };
    }

    pub fn reset(&mut self) {
        self.state = match self.state {
            BallProgress::Done => BallProgress::None,
            x => x,
        }
    }

    pub fn is_done(&self) -> bool {
        match self.state {
            BallProgress::Done => true,
            _ => false,
        }
    }

    pub fn idempotent_start(&mut self) {
        use BallProgress::*;
        self.state = match self.state {
            None => Intaking,
            x => x,
        }
    }

    pub fn is_in_carriage(&self) -> bool {
        match self.state {
            BallProgress::CarriageSecure | BallProgress::Outtaking(_) => true,
            _ => false,
        }
    }

    pub fn process_sensors(&mut self, elev_ready: bool) -> Result<(), HalCtreError> {
        dbg!(self.state);
        use BallProgress::*;
        // TODO add backwards transitions for unjam
        self.state = match self.state {
            None => None,
            Intaking => {
                if !self.gates.0.get()? {
                    Inside
                } else {
                    Intaking
                }
            }
            // TODO should be handled here to be on the safer side?
            Inside => {
                if !self.gates.1.get()? {
                    Queued
                } else {
                    Inside
                }
            }
            Queued => {
                if elev_ready {
                    CarriageVolatile
                } else {
                    Queued
                }
            }
            CarriageVolatile => {
                if !self.gates.2.get()? {
                    CarriageSecure
                } else {
                    CarriageVolatile
                }
            }
            CarriageSecure => CarriageSecure,
            Outtaking(ticks) => {
                if ticks > 0 {
                    Outtaking(ticks - 1)
                } else {
                    Done
                }
            }
            Done => Done,
        };
        Ok(())
    }

    pub fn write_outs(&self, outs: &mut PeriodicOuts) {
        use BallProgress::*;
        match self.state {
            None | Done => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = 0.0;
            }
            Intaking => {
                outs.intk_pnm = IntakeExt::Ext.into();
                outs.intk_pct = CHAN_INTAKE_COMMAND;
            }
            Inside => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = CHAN_CONVEY_COMMAND;
            }
            Queued => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = 0.0;
            }
            CarriageVolatile => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = CHAN_TRANSFER_COMMAND;
                outs.outk_pct = OUTK_INTK_COMMAND;
            }
            CarriageSecure => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = 0.0;
            }
            Outtaking(_) => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.outk_pct = OUTK_OUTK_COMMAND;
            }
        }
    }
}
