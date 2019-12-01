// no invariants to uphold for these
// they're just unsafe because theyre intrinsics
macro_rules! likely {
    ($($arg:tt)*) => {
        unsafe { std::instrinsics::likely($($arg)*) }
    };
}

macro_rules! unlikely {
    ($($arg:tt)*) => {
        unsafe { std::instrinsics::unlikely($($arg)*) }
    };
}
