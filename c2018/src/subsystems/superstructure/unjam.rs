use self::UnjamState::*;
use super::IntakeExt;

// TODO tune
const UNJAM_CHAN_COMMAND: f64 = 1.0;
const UNJAM_OUTK_COMMAND: f64 = 1.0;
const UNJAM_SWITCH_CYCLES: u16 = 100;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum UnjamState {
    Disabled,
    Forward(u16),
    Reverse(u16),
}

impl UnjamState {
    pub fn process(&mut self) {
        *self = match *self {
            Disabled => Disabled,
            Forward(x) if x > 0 => Forward(x - 1),
            Reverse(x) if x > 0 => Reverse(x - 1),
            Forward(_) => Reverse(UNJAM_SWITCH_CYCLES),
            Reverse(_) => Forward(UNJAM_SWITCH_CYCLES),
        }
    }

    pub fn set_enabled(&mut self, enable: bool) {
        if enable && *self == Disabled {
            *self = Forward(UNJAM_SWITCH_CYCLES)
        } else if !enable {
            *self = Disabled
        }
    }

    pub fn write_outs(&self, outs: &mut super::PeriodicOuts) {
        match self {
            Disabled => (),
            Forward(_) => {
                outs.intk_pct = UNJAM_CHAN_COMMAND;
                outs.outk_pct = UNJAM_OUTK_COMMAND;
                outs.intk_pnm = IntakeExt::Ext.into()
            }
            Reverse(_) => {
                outs.intk_pct = -UNJAM_CHAN_COMMAND;
                outs.outk_pct = -UNJAM_OUTK_COMMAND;
                outs.intk_pnm = IntakeExt::Ext.into()
            }
        }
    }
}
