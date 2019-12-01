use crate::util::unlikely;
use libc::{clock_gettime, timespec, CLOCK_MONOTONIC};
pub use std::time::{Duration, Instant};

#[inline]
fn timespec_to_duration(t: timespec) -> Duration {
    Duration::from_secs(t.tv_sec as _) + Duration::from_nanos(t.tv_nsec as _)
}

// TODO: benchmark overhead vs std::time::Instant (has more indirection, but inlining).
/// Duration since some unspecified point. Uses CLOCK_MONOTONIC, but allows introspection of time values unlike `std::time::Instant`.
#[inline]
fn monotonic() -> Duration {
    let t: timespec = unsafe { std::mem::zeroed() };
    let res = unsafe { clock_gettime(CLOCK_MONOTONIC, &mut t) };
    if unlikely!(res != 0) {
        crate::die::die_with_errno!("clock_gettime");
    }
    return timespec_to_duration(t);
}
