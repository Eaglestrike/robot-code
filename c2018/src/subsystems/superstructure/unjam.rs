use super::IntakeExt;
// TODO tune
const UNJAM_CHAN_COMMAND: f64 = 1.0;
const UNJAM_OUTK_COMMAND: f64 = 1.0;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum UnjamState {
    Disabled,
    Forward(u16),
    Reverse(u16),
}

const UNJAM_SWITCH_CYCLES: u16 = 100;
impl UnjamState {
    pub fn process(&mut self) {
        use UnjamState::*;
        *self = match *self {
            Disabled => Disabled,
            Forward(x) => {
                if x > 0 {
                    Forward(x - 1)
                } else {
                    Reverse(UNJAM_SWITCH_CYCLES)
                }
            }
            Reverse(x) => {
                if x > 0 {
                    Reverse(x - 1)
                } else {
                    Forward(UNJAM_SWITCH_CYCLES)
                }
            }
        }
    }
    pub fn set_enabled(&mut self, en: bool) {
        use UnjamState::*;
        *self = match en {
            true => match *self {
                Disabled => Forward(UNJAM_SWITCH_CYCLES),
                x => x,
            },
            false => Disabled,
        }
    }

    pub fn write_outs(&self, outs: &mut super::PeriodicOuts) {
        use UnjamState::*;
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
                outs.intk_pnm = IntakeExt::Retr.into()
            }
        }
    }
}
