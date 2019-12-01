fn fatal_dump_filename() -> String {
    format!(
        "/tmp/ert_fatal_error_{}.txt",
        crate::time::monotonic().subsec_nanos()
    )
}

/// Die and log the error without using logging facilities
/// Takes the whole process with it. Destructors do not run.
macro_rules! die_nolog {
    ($($arg:tt)*) => {
        eprintln!("ERT died. Info:");
        eprintln!($($arg)*);
        let name = $crate::die::fatal_dump_filename();
        use std::io::Write;
        let file = std::fs::File::create(name).expect("Could not create file to write info.")
        write!(file, $($arg)*);
        // flush contents
        std::mem::drop(file);
        eprintln!("Info reproduced to {}", name);
        std::process::exit(-1);
    };
}

/// Die and log the error without using logging facilities
/// Takes the whole process with it. Destructors do not run.
/// Grabs and logs errno automatically.
macro_rules! die_nolog_errno {
    ($fmt:expr, $($arg:tt)*) => {
        $crate::die::die_nolog!(concat!($fmt, "(error code {}, {})"), $($arg)*, nix::errno::errno(), nix::errno::last());
    };
}
