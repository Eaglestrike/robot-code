//! Spawning threads and pthread extensions

use libc::*;

// Rust's default is 2MB, we prolly don't need that
pub const DEFAULT_STACK_SIZE: usize = 512 * 1024;

pub fn thread_with_stack_size(name: impl Into<String>, stack_size: usize) -> std::thread::Builder {
    std::thread::Builder::new()
        .name(name.into())
        .stack_size(stack_size)
}

pub fn thread(name: impl Into<String>) -> std::thread::Builder {
    thread_with_stack_size(name, PTHREAD_STACK_MIN + DEFAULT_STACK_SIZE)
}
