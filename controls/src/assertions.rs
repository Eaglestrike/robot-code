#[macro_export]
macro_rules! dbg_isfinite {
    ($a:expr) => {
        debug_assert!(
            ($a).is_finite(),
            "Assertion failed: a.is_finite()\n{}\n{}",
            stringify!($a),
            $a,
        );
    };
}

#[macro_export]
macro_rules! dbg_lt {
    ($left:expr, $right:expr) => {
        debug_assert!(
            $left < $right,
            "Assertion failed: left < right\n{} < {}\n{} < {}",
            stringify!($left),
            stringify!($right),
            $left,
            $right
        );
    };
}

#[macro_export]
macro_rules! dbg_gt {
    ($left:expr, $right:expr) => {
        debug_assert!(
            $left > $right,
            "Assertion failed: left > right\n{} > {}\n{} > {}",
            stringify!($left),
            stringify!($right),
            $left,
            $right
        );
    };
}

#[macro_export]
macro_rules! dbg_eq {
    ($left:expr, $right:expr) => {
        debug_assert!(
            $left == $right,
            "Assertion failed: left == right\n{} == {}\n{} == {}",
            stringify!($left),
            stringify!($right),
            $left,
            $right
        );
    };
}

#[macro_export]
macro_rules! dbg_near {
    ($left:expr, $right:expr) => {
        ($left - $right).abs() < 1e-6,
        debug_assert!(
            $left == $right,
            "Assertion failed: left == right\n{} == {}\n{} == {}",
            stringify!($left),
            stringify!($right),
            $left,
            $right
        );
    };

    ($left:expr, $right:expr, $delta:expr) => {
        debug_assert!(
            ($left - $right).abs() < $delta,
            "Assertion failed: left == right within delta\n{} == {}; delta: {}\n{} == {}; delta: {}",
            stringify!($left),
            stringify!($right),
            stringify!($delta),
            $left,
            $right,
            $delta
        );
    };
}
