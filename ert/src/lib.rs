//! # Eaglestrike Run-Time
//! Basically an inferior clone of AOS, using threads instead of processes.

#[cfg_attr(feature_flaggable, feature(core_intrinsics))]
#[macro_use]
pub mod die;
pub mod init;
pub mod logging;
pub mod queue;
pub mod sync;
pub mod thread;
pub mod time;
#[macro_use]
pub mod util;

#[doc(hidden)]
// reexport symbols for the logging macros to use
pub mod log_rexp {
    pub use super::logging::rexp::*;
}

#[cfg(test)]
mod test_prelude {
    pub use rusty_fork::{rusty_fork_id, rusty_fork_test, rusty_fork_test_name};
}
