//! Handle failures we can't or don't want to recover from.
//! Unlike AOS, which handles logging in another process,
//! these will take down the whole system.
//! Thread-level issues should just be panics.
//! A wrapper that logs then panics is planned.

/// Implementation detail.
#[doc(hidden)]
pub fn fatal_dump_filename() -> String {
    format!("/tmp/ert_fatal_error_{}.txt", unsafe { libc::rand() })
}

/// Die and log the error without using logging facilities
/// Takes the whole process with it. Destructors do not run.
#[macro_export]
macro_rules! die {
    ($($arg:tt)*) => {
        let info = concat!("At ", std::file!(), ":", std::line!());
        eprintln!("ERT encountered a fatal error! Info to follow:");
        eprintln!("{}", info);
        eprintln!($($arg)*);
        let name = $crate::die::fatal_dump_filename();
        use std::io::Write;
        let res = std::fs::File::create(&name).and_then(|mut file| {
            writeln!(file, "{}", info);
            writeln!(file, $($arg)*)
        });
        if res.is_ok() {
            eprintln!("Info reproduced to {}", &name);
        } else {
            eprintln!("Failed to write error info to {}: {:?}", &name, res);
        }
        std::process::exit(-1);
    };
}

/// Die and log the error without using logging facilities
/// Takes the whole process with it. Destructors do not run.
/// Grabs and logs errno automatically.
#[macro_export]
macro_rules! die_with_errno {
    ($fmt:expr, $($arg:tt)*) => {
        $crate::die!(concat!($fmt, " (caused by error code {}, {})"), $($arg)*, nix::errno::errno(), nix::errno::Errno::last());
    };
    ($fmt:expr) => {
        $crate::die!(concat!($fmt, " (caused by error code {}, {})"), nix::errno::errno(), nix::errno::Errno::last());
    };
}

#[macro_export]
macro_rules! panic_with_errno {
    ($fmt:expr, $($arg:tt)*) => {
        panic!(concat!($fmt, " (caused by error code {}, {})"), $($arg)*, nix::errno::errno(), nix::errno::Errno::last());
    };
    ($fmt:expr) => {
        panic!(concat!($fmt, " (caused by error code {}, {})"), nix::errno::errno(), nix::errno::Errno::last());
    };
}
