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

// TODO a good sleep method

pub use std::thread::Result as ThreadResult;

pub trait TryJoin<T>: Sized {
    fn try_join(self) -> Result<ThreadResult<T>, Self>;

    fn join_timeout(self, timeout: crate::time::Duration) -> Result<ThreadResult<T>, Self>;
}

#[allow(unused_imports)]
use std::os::unix::thread::JoinHandleExt;

// TODO get these np joins into libc so we can get some better error messages with conditional compilation
// GNU extensions woot woot
extern "C" {
    fn pthread_tryjoin_np(native: pthread_t, value: *mut *mut c_void) -> c_int;
    fn pthread_timedjoin_np(
        native: pthread_t,
        value: *mut *mut c_void,
        abstime: *const timespec,
    ) -> c_int;
}

// Implementing TryJoin for generic data would require re-implementing std's threading functionality,
// which contains a methods used to get return values that is incompatible with raw pthreads,
// and pthreads doesnt like multiple joining meaning that there's no way to get a return values
// after a successful call to either of the tryjoin methods
// Until I can figure out a reasonable solution, here's the half-feature for void-returning threads
impl TryJoin<()> for std::thread::JoinHandle<()> {
    /// Cannot detect panics! If a thread panics, this will return as if it returned normally.
    fn try_join(self) -> Result<ThreadResult<()>, Self> {
        let handle = self.as_pthread_t();
        let ret = unsafe { pthread_tryjoin_np(handle, std::ptr::null_mut()) };
        match ret {
            // thread done, join it
            0 => Ok(Ok(())),
            // thread not done, return the JoinHandle
            EBUSY => Err(self),
            // Other error (from pthread_join)
            // These should all be prevented statically by single ownership of a
            // JoinHandle, so kill the process
            err => {
                use nix::errno;
                crate::die!("Error encountered in TryJoin::try_join\nThread (name {:?}) couldn't join thread (name {:?}), caused by {}, {}",
                std::thread::current().name(), self.thread().name(), err, errno::from_i32(err));
            }
        }
    }

    /// Cannot detect panics! If a thread panics, this will return as if it returned normally.
    /// Since `pthread_timedjoin_np` is broken in glibc, this is currently implemented with a
    /// busywait on `try_join`.
    fn join_timeout(self, timeout: crate::time::Duration) -> Result<ThreadResult<()>, Self> {
        #![allow(unreachable_code)]
        // unimplemented!("pthread_timedjoin_np is broken in glibc");
        let start = std::time::Instant::now();
        let mut faux_self = self;
        while start.elapsed() < timeout {
            match faux_self.try_join() {
                Err(s) => {
                    faux_self = s;
                    std::thread::yield_now()
                }
                x => return x,
            }
        }
        return Err(faux_self);
        // dead code (glibc impl) below
        let handle = self.as_pthread_t();
        let timespec = crate::time::duration_to_timespec(timeout);
        dbg!(timespec);
        let ret = unsafe { pthread_timedjoin_np(handle, std::ptr::null_mut(), &timespec) };
        match ret {
            // thread done, join it
            0 => Ok(Ok(())),
            // thread not done or timeout, return the JoinHandle
            err @ EBUSY | err @ ETIMEDOUT => {
                use nix::errno;
                dbg!(errno::from_i32(err));
                Err(self)
            }
            // our timespec was malformed, should be impossible coming from a Duration
            EINVAL => unreachable!(),
            // Other error (from pthread_join)
            // These should all be prevented statically by single ownership of a
            // JoinHandle, so kill the process
            err => {
                use nix::errno;
                crate::die!("Error encountered in TryJoin::join_timeout\nThread (name {:?}) couldn't join thread (name {:?}), caused by {}, {}",
                std::thread::current().name(), self.thread().name(), err, errno::from_i32(err));
            }
        }
    }
}

#[cfg(test)]
mod tryjoin_test {

    use super::*;
    use std::sync::{
        atomic::{AtomicUsize, Ordering},
        Arc,
    };

    #[derive(Debug, Clone)]
    struct BusyGate(Arc<AtomicUsize>);

    impl BusyGate {
        pub fn new() -> Self {
            Self(Arc::new(AtomicUsize::new(0)))
        }

        pub fn busywait(&self) {
            while self.0.load(Ordering::Acquire) == 0 {}
            std::thread::sleep(std::time::Duration::from_millis(10));
        }

        pub fn open(&self) {
            self.0.store(1, Ordering::Release);
            std::thread::yield_now();
            std::thread::sleep(std::time::Duration::from_millis(10));
            std::thread::yield_now();
        }
    }

    #[test]
    fn tryjoin_no_early_return() {
        let ping = BusyGate::new();
        let ping_ = ping.clone();
        let handle = thread("tryjoin_basics_child")
            .spawn(move || {
                ping_.busywait();
            })
            .unwrap();
        let handle = handle.try_join().unwrap_err();
        let timeout = std::time::Duration::from_millis(100);
        let start = std::time::Instant::now();
        let handle = handle.join_timeout(timeout).unwrap_err();
        let el = start.elapsed();
        assert_eq!(el.as_secs(), 0);
        dbg!(el);
        assert!(el.subsec_millis() > 90);
        assert!(el.subsec_millis() < 110);
        ping.open();
        handle.join().unwrap();
    }

    #[test]
    fn tryjoin_try_join() {
        let ping = BusyGate::new();
        let pong = BusyGate::new();
        let ping_ = ping.clone();
        let pong_ = pong.clone();
        let handle = thread("tryjoin_basics_child")
            .spawn(move || {
                ping_.busywait();
                pong_.open();
            })
            .unwrap();
        let handle = handle.try_join().unwrap_err();
        ping.open();
        pong.busywait();
        match handle.try_join() {
            Err(handle) => {
                // try avoid spurious failures due to scheduling
                std::thread::sleep(std::time::Duration::from_millis(250));
                handle.try_join()
            }
            x => x,
        }
        .unwrap()
        .unwrap();
    }

    #[test]
    fn tryjoin_join_timeout() {
        let ping = BusyGate::new();
        let pong = BusyGate::new();
        let ping_ = ping.clone();
        let pong_ = pong.clone();
        let handle = thread("tryjoin_basics_child")
            .spawn(move || {
                ping_.busywait();
                pong_.open();
            })
            .unwrap();
        let timeout = std::time::Duration::from_millis(100);
        let handle = handle.join_timeout(timeout).unwrap_err();
        ping.open();
        pong.busywait();
        dbg!(handle.join_timeout(timeout)).unwrap().unwrap();
    }
}
