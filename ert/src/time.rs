use libc::{clock_gettime, timespec, CLOCK_MONOTONIC};
pub use std::time::{Duration, Instant};

#[inline]
pub(crate) fn timespec_to_duration(t: timespec) -> Duration {
    Duration::from_secs(t.tv_sec as _) + Duration::from_nanos(t.tv_nsec as _)
}

#[inline]
pub(crate) fn duration_to_timespec(t: Duration) -> timespec {
    timespec {
        tv_sec: t.as_secs() as _,
        tv_nsec: t.subsec_nanos().into(),
    }
}

#[cfg(test)]
#[test]
fn duration_conversions() {
    fn test(t: Duration) {
        let c = timespec_to_duration(duration_to_timespec(t));
        assert_eq!(t, c);
        let ts1 = duration_to_timespec(t);
        let ts2 = duration_to_timespec(timespec_to_duration(ts1));
        assert_eq!(ts1, ts2);
    }
    [
        Duration::from_millis(1043120),
        Duration::from_micros(10121),
        Duration::from_nanos(1123),
        Duration::from_secs(121),
    ]
    .into_iter()
    .for_each(|d| test(*d));
}

// TODO: benchmark overhead vs std::time::Instant (has more indirection, but inlining).
/// Duration since some unspecified point. Uses CLOCK_MONOTONIC, but allows introspection of time values unlike `std::time::Instant`.
#[inline]
pub fn monotonic() -> Duration {
    let mut t: timespec = unsafe { std::mem::zeroed() };
    let res = unsafe { clock_gettime(CLOCK_MONOTONIC, &mut t) };
    if crate::unlikely!(res != 0) {
        crate::die_with_errno!("clock_gettime");
    }
    return timespec_to_duration(t);
}
