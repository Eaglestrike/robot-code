use super::HalCtreError;
use super::IntakeExt;
use super::PeriodicOuts;
use wpilib::{dio::DigitalInput, HalResult};

// TODO tune
const CHAN_INTAKE_COMMAND: f64 = 0.8;
const CHAN_CONVEY_COMMAND: f64 = 0.6;
const CHAN_TRANSFER_COMMAND: f64 = 1.0;
const OUTK_INTK_COMMAND: f64 = 0.5;
const OUTK_OUTK_COMMAND: f64 = 1.0;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum BallProgress {
    None,
    Intaking,
    Inside,
    Queued,
    CarriageVolatile,
    CarriageSecure,
    Outtaking,
    Done,
}

#[derive(Debug)]
pub struct Channel {
    state: BallProgress,
    gates: (DigitalInput, DigitalInput, DigitalInput),
}

use crate::config::superstructure::{GATE1, GATE2, GATE3};
use BallProgress::{Inside, Intaking, Outtaking};

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
        if self.state == BallProgress::Intaking {
            self.state = BallProgress::None;
            return true;
        }
        false
    }

    pub fn try_init_outk(&mut self) -> bool {
        if self.state == BallProgress::CarriageSecure {
            self.state = BallProgress::Outtaking;
            return true;
        }
        false
    }

    pub fn try_stop_outk(&mut self) -> bool {
        if self.state == BallProgress::Outtaking {
            self.state = BallProgress::Done;
            return true;
        }
        false
    }

    pub fn reset(&mut self) {
        if self.state == BallProgress::Done {
            self.state = BallProgress::None;
        }
    }

    pub fn is_done(&self) -> bool {
        self.state == BallProgress::Done
    }

    pub fn idempotent_start(&mut self) {
        if self.state == BallProgress::None {
            self.state = BallProgress::Intaking;
        }
    }

    pub fn is_in_carriage(&self) -> bool {
        self.state == BallProgress::CarriageSecure || self.state == BallProgress::Outtaking
    }

    pub fn force_abort(&mut self) {
        self.state = BallProgress::None;
    }

    pub fn process_sensors(&mut self, elev_ready: bool) -> Result<(), HalCtreError> {
        // dbg!(self.state);
        use BallProgress::*;
        // TODO add backwards transitions for unjam
        // dbg!((
        //     self.gates.0.get()?,
        //     self.gates.1.get()?,
        //     self.gates.2.get()?
        // ));
        self.state = match self.state {
            Intaking if !self.gates.0.get()? => Inside,
            // TODO should be handled here to be on the safer side?
            Inside if !self.gates.1.get()? => Queued,
            Queued if elev_ready => CarriageVolatile,
            CarriageVolatile if !self.gates.2.get()? => CarriageSecure,
            x => x,
        };
        Ok(())
    }

    pub fn write_outs(&self, outs: &mut PeriodicOuts) {
        use BallProgress::*;
        match self.state {
            None | Done | Queued | CarriageSecure => {
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
            CarriageVolatile => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.intk_pct = CHAN_TRANSFER_COMMAND;
                outs.outk_pct = OUTK_INTK_COMMAND;
            }
            Outtaking => {
                outs.intk_pnm = IntakeExt::Retr.into();
                outs.outk_pct = OUTK_OUTK_COMMAND;
            }
        }
    }
}
