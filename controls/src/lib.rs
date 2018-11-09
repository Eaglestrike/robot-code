#![feature(test)]
#![feature(const_fn)]

pub mod integration;

// re-exports
pub mod approx {
    pub use approx::*;
}

#[macro_use]
pub mod units {
    pub use dimensioned::si::*;
    use dimensioned::typenum::{tarr, N1, N2, P1, Z0};
    /// Used for Kd in PID loops
    pub type VoltSecondPerMeter<V> = SI<V, tarr![P1, P1, N2, N1, Z0, Z0, Z0]>; // also Newtons per Amp

    #[macro_export]
    macro_rules! const_unit {
        ($val:expr) => {
            $crate::units::SI {
                value_unsafe: $val,
                _marker: std::marker::PhantomData,
            }
        };
    }
}

use crate::units as un;
use serde::Serialize;
use std::fs::File;
use std::path::Path;

pub trait HarnessAble {
    /// A type representing the state of the system
    type State: Copy;
    /// A type that represents the output of the controller
    type ControlResponse: Copy;
    /// A type holding all the data logged to a csv once the test completes
    type LogData: Serialize;
    /// Physically simulates the system over the time where the control response is constant
    fn sim_time(s: Self::State, r: Self::ControlResponse, dur: un::Second<f64>) -> Self::State;
    /// The duration of one period for physics simulation
    const SIMUL_DT: un::Second<f64>;
    /// The interval between control response updates
    const CONTROL_DT: un::Second<f64>;
}

/// Shims a physically simulated state into simulated sensors passed to the control loop.
///
/// Can be used to simulate things like encoder offsets, failing sensors.
pub trait StateShim<SYS>
where
    SYS: HarnessAble,
{
    /// The output of the shimmed controller based on the current physical state
    fn update(&mut self, state: SYS::State) -> SYS::ControlResponse;

    /// Return data to log at the current time
    fn log_dat(
        &mut self,
        state: SYS::State,
        response: SYS::ControlResponse,
        time: un::Second<f64>,
    ) -> SYS::LogData;

    /// Include safety assertions about the physical state. Will be called in tests
    fn assert(&mut self, _state: SYS::State) {}
}

pub struct SimulationHarness<SYS, SHIM>
where
    SYS: HarnessAble,
    SHIM: StateShim<SYS>,
{
    shim: SHIM,
    state: SYS::State,
    time: un::Second<f64>,
    log_every: u32,
    csv: Option<csv::Writer<File>>,
    log: Vec<SYS::LogData>,
}

impl<SYS, SHIM> SimulationHarness<SYS, SHIM>
where
    SYS: HarnessAble,
    SHIM: StateShim<SYS>,
{
    pub fn new(shim: SHIM, initial: SYS::State, log_every: u32) -> Self {
        Self {
            shim,
            state: initial,
            time: 0. * un::S,
            log_every,
            csv: None,
            log: Vec::new(),
        }
    }

    pub fn use_csv<P: AsRef<Path> + std::fmt::Debug>(&mut self, path: P) {
        match path.as_ref().parent() {
            Some(dir) => std::fs::create_dir_all(dir).expect(&format!(
                "Could not create parent directory of csv {:?}",
                path
            )),
            _ => (),
        };

        self.csv = Some(
            csv::WriterBuilder::new()
                .delimiter(b' ')
                .from_path(path)
                .expect("Could not create csv writer"),
        )
    }

    pub fn shim(&self) -> &SHIM {
        &self.shim
    }

    pub fn shim_mut(&mut self) -> &mut SHIM {
        &mut self.shim
    }

    pub fn run_time(&mut self, time: un::Second<f64>) -> SYS::State {
        let mut elapsed = 0. * un::S;
        let mut count = 0;
        while elapsed < time {
            let response = self.shim.update(self.state);
            self.state = SYS::sim_time(self.state, response, SYS::CONTROL_DT);
            self.shim.assert(self.state);
            elapsed += SYS::CONTROL_DT;
            self.time += SYS::CONTROL_DT;
            count += 1;
            if count >= self.log_every {
                self.log
                    .push(self.shim.log_dat(self.state, response, self.time));
                count = 0;
            }
        }
        return self.state;
    }
}

// implement csv output in Drop so that it runs even on test failures
impl<SYS, SHIM> Drop for SimulationHarness<SYS, SHIM>
where
    SYS: HarnessAble,
    SHIM: StateShim<SYS>,
{
    fn drop(&mut self) {
        match self.csv {
            Some(ref mut wtr) => self.log.iter().for_each(|r| {
                wtr.serialize(r)
                    .unwrap_or_else(|_| println!("ERROR: Record serialization failed!"))
            }),
            _ => (),
        }
    }
}

pub mod util {
    pub fn clamp<T: PartialOrd>(a: T, lower: T, upper: T) -> T {
        debug_assert!(lower < upper);
        if a < lower {
            lower
        } else if a > upper {
            upper
        } else {
            a
        }
    }
}

#[macro_use]
mod assertions;
