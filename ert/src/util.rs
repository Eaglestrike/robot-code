// no invariants to uphold for these
// they're just unsafe because theyre intrinsics

#[cfg(feature_flaggable)]
#[macro_export]
macro_rules! likely {
    ($($arg:tt)*) => {
        unsafe { std::intrinsics::likely($($arg)*) }
    };
}

#[cfg(not(feature_flaggable))]
#[macro_export]
macro_rules! likely {
    ($($arg:tt)*) => { $($arg)* };
}

#[cfg(feature_flaggable)]
#[macro_export]
macro_rules! unlikely {
    ($($arg:tt)*) => {
        unsafe { std::intrinsics::unlikely($($arg)*) }
    };
}

#[cfg(not(feature_flaggable))]
#[macro_export]
macro_rules! unlikely {
    ($($arg:tt)*) => { $($arg)* };
}
