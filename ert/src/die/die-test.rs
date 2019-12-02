use ert::{die, die_with_errno};
use libc::{clock_settime, CLOCK_MONOTONIC};
use std::env::args;
use std::process::exit;

// TODO write a real test-harness for this:
// https://users.rust-lang.org/t/how-do-you-test-binaries-not-libraries/9554/8

// dummy struct that's not Copy, but is Debug
#[derive(Debug)]
struct DummyStruct(i32);

fn main() {
    let number = args().nth(1).unwrap().parse::<u32>().unwrap();
    // do something that will definitely fail
    let mut timespec = unsafe { std::mem::zeroed() };
    let ret = unsafe { clock_settime(CLOCK_MONOTONIC, &mut timespec) };
    if ret == 0 {
        println!("Test Malfunction");
        eprintln!("Test Malfunction");
        exit(0);
    }
    let info = DummyStruct(114);
    match number {
        0 => {
            die!("{} {:?}", "format", info);
        }
        1 => {
            die_with_errno!("clock_gettime");
        }
        2 => {
            die_with_errno!("clock_gettime {} {:?}", "formatTestString", info);
        }
        255 => {
            // test ownership behavior by whether this compiles
            if number == 254 {
                // rustc can't do this analysis, so it looks like a conditional
                die!("{} {:?} {:?}", "format", info, info);
            }
            std::mem::drop(info);
        }
        _ => (),
    }
    exit(0);
}
