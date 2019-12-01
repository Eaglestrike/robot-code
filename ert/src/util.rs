// no invariants to uphold for these
// they're just unsafe because theyre intrinsics

#[cfg(not(RUSTC_IS_STABLE))]
#[macro_export]
macro_rules! likely {
    ($($arg:tt)*) => {
        unsafe { std::intrinsics::likely($($arg)*) }
    };
}

#[cfg(RUSTC_IS_STABLE)]
#[macro_export]
macro_rules! likely {
    ($($arg:tt)*) => { $($arg)* };
}

#[cfg(not(RUSTC_IS_STABLE))]
#[macro_export]
macro_rules! unlikely {
    ($($arg:tt)*) => {
        unsafe { std::intrinsics::unlikely($($arg)*) }
    };
}

#[cfg(RUSTC_IS_STABLE)]
#[macro_export]
macro_rules! unlikely {
    ($($arg:tt)*) => { $($arg)* };
}
