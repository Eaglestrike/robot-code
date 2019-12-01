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
        eprintln!("ERT encountered a fatal error! Info:");
        eprintln!($($arg)*);
        let name = $crate::die::fatal_dump_filename();
        use std::io::Write;
        let res = std::fs::File::create(&name).and_then(|mut file| write!(file, $($arg)*));
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
        $crate::die!(concat!($fmt, "(error code {}, {})"), $($arg)*, nix::errno::errno(), nix::errno::last());
    };
    ($fmt:expr) => {
        $crate::die!(concat!($fmt, "(error code {}, {})"), nix::errno::errno(), nix::errno::Errno::last());
    };
}
