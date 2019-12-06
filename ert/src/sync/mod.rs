//! Exports nice interfaces to low level sync stuff, primarily the aos mutexes and futexes

use aos_sync as raw;

use nix::errno::errno;
use std::cell::UnsafeCell;

// does this need to be heap allocated to deal with move semantics?
// I would think moving the mutex would break it, but then again
// rust statically guarantees no moves unless we have exclusive ownership
// any reasonable use (Arc<AosMutexRaw>) will require it gets moved.

/// PI Mutex
///
/// If you want to guard a single Rust data structure, and not something more complicated, prefer the `AosMutex` type.
///
/// NOT Re-entrant
///
/// NOT Data-race safe
///
/// Callers MUST ensure that this is not moved while there are any locks on it.
#[derive(Debug)]
#[repr(transparent)]
pub struct AosMutexRaw {
    m: UnsafeCell<raw::aos_mutex>,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
#[must_use]
pub enum LockSuccessReason {
    Unlocked,
    OwnerDied,
}
impl LockSuccessReason {
    fn ok(self) {}
}

impl AosMutexRaw {
    pub fn new() -> Self {
        Self {
            m: unsafe { std::mem::zeroed() },
        }
    }

    pub fn lock(&self) -> LockSuccessReason {
        let ret = unsafe { raw::mutex_grab(self.m.get()) };
        match ret {
            0 => LockSuccessReason::Unlocked,
            1 => LockSuccessReason::OwnerDied,
            _ => unreachable!("AosMutexRaw mutex_grab failed with {}", ret),
        }
    }

    pub fn lock_signals_fail(&self) -> Result<LockSuccessReason, ()> {
        let ret = unsafe { raw::mutex_lock(self.m.get()) };
        match ret {
            0 => Ok(LockSuccessReason::Unlocked),
            1 => Ok(LockSuccessReason::OwnerDied),
            2 => Err(()),
            _ => unreachable!("AosMutexRaw mutex_grab failed with {}", ret),
        }
    }

    pub fn unlock(&self) {
        unsafe { raw::mutex_unlock(self.m.get()) }
    }

    pub fn try_lock(&self) -> Result<LockSuccessReason, ()> {
        let ret = unsafe { raw::mutex_trylock(self.m.get()) };
        match ret {
            0 => Ok(LockSuccessReason::Unlocked),
            1 => Ok(LockSuccessReason::OwnerDied),
            4 => Err(()),
            _ => unreachable!("AosMutexRaw mutex_trylock failed with {}", ret),
        }
    }

    pub fn is_locked(&self) -> bool {
        unsafe { raw::mutex_islocked(self.m.get()) }
    }
}

impl Drop for AosMutexRaw {
    fn drop(&mut self) {
        if self.is_locked() {
            self.unlock()
        }
    }
}

unsafe impl Send for AosMutexRaw {}
unsafe impl Sync for AosMutexRaw {}

unsafe impl lock_api::RawMutex for AosMutexRaw {
    type GuardMarker = lock_api::GuardNoSend;

    const INIT: AosMutexRaw = Self {
        m: UnsafeCell::new(raw::aos_mutex {
            next: 0,
            previous: 0 as *mut _,
            futex: 0,
        }),
    };

    #[inline]
    fn lock(&self) {
        self.lock().ok()
    }

    #[inline]
    fn try_lock(&self) -> bool {
        self.try_lock().is_ok()
    }

    fn unlock(&self) {
        self.unlock()
    }
}

/// Type-safe PI Mutex
///
/// Should be safe to move, as you can only move it when no one could lock it, proved by lifetime analysis.
/// Any unsafe shenanigans will break this invariant, though.
pub type AosMutex<T> = lock_api::Mutex<AosMutexRaw, T>;
pub type AosMutexGuard<'a, T> = lock_api::MutexGuard<'a, AosMutexRaw, T>;

#[cfg(test)]
mod aos_mutex_test {
    use super::*;
    #[test]
    fn raw_mutex_const_init_eq_memset() {
        const SIZE: usize = std::mem::size_of::<AosMutexRaw>();
        let init: [u8; SIZE] =
            unsafe { std::mem::transmute(<AosMutexRaw as lock_api::RawMutex>::INIT) };
        let other: [u8; SIZE] = unsafe { std::mem::transmute(AosMutexRaw::new()) };
        assert_eq!(init, other);
    }
    // TODO add actual functionality tests for AosMutex
}

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum FutexWakeReason {
    SignalInterrupt,
    Timeout,
    Errno(i32),
}

#[derive(Debug)]
#[repr(transparent)]
pub struct AosFutexNotifier {
    m: UnsafeCell<raw::aos_futex>,
}

/// A lock-free broadcast notifier based on `aos_futex`
impl AosFutexNotifier {
    pub fn new() -> Self {
        Self {
            m: unsafe { std::mem::zeroed() },
        }
    }

    pub fn wait(&self) -> Result<(), FutexWakeReason> {
        let ret = unsafe { raw::futex_wait(self.m.get()) };
        match ret {
            0 => {
                self.unset();
                Ok(())
            }
            1 => Err(FutexWakeReason::SignalInterrupt),
            _ => Err(FutexWakeReason::Errno(errno())),
        }
    }

    pub fn wait_timeout(&self, timeout: std::time::Duration) -> Result<(), FutexWakeReason> {
        let ts = raw::timespec {
            tv_sec: timeout.as_secs() as _,
            tv_nsec: timeout.subsec_nanos().into(),
        };
        let ret = unsafe { raw::futex_wait_timeout(self.m.get(), &ts) };
        match ret {
            0 => Ok(()),
            1 => Err(FutexWakeReason::SignalInterrupt),
            2 => Err(FutexWakeReason::Timeout),
            _ => Err(FutexWakeReason::Errno(errno())),
        }
    }

    /// Returns was not set before. In other words, returns if the value was changed.
    #[inline(always)]
    fn unset(&self) -> bool {
        unsafe { raw::futex_unset(self.m.get()) != 0 }
    }

    pub fn notify(&self) -> Result<(), i32> {
        let ret = unsafe { raw::futex_set(self.m.get()) };
        std::sync::atomic::compiler_fence(std::sync::atomic::Ordering::SeqCst);
        self.unset();
        if ret < 0 {
            Err(errno())
        } else {
            Ok(())
        }
    }
}

unsafe impl Send for AosFutexNotifier {}
unsafe impl Sync for AosFutexNotifier {}

#[cfg(test)]
mod aos_notifier_tests {
    use super::*;
    use std::sync::atomic::AtomicU32;
    use std::sync::atomic::Ordering::SeqCst;
    use std::time::Duration;
    fn atomic_busywait_timeout(a: &AtomicU32, v: u32, timeout: Duration) {
        use std::time::Instant;
        let start = Instant::now();
        loop {
            if a.load(SeqCst) == v {
                std::thread::sleep(Duration::from_millis(10));
                assert_eq!(a.load(SeqCst), v);
                break;
            }
            if start.elapsed() > timeout {
                panic!("Atomic busywait timeout");
            }
        }
    }
    #[test]
    fn aos_notifier_many_threads() {
        use std::sync::Arc;
        use std::thread;

        let mut t = Vec::new();
        let ct = Arc::new(AtomicU32::new(0));
        let notif = Arc::new(AosFutexNotifier::new());
        let num_threads = 500;
        for _ in 0..num_threads {
            let n = Arc::clone(&notif);
            let c = Arc::clone(&ct);
            let h = thread::spawn(move || {
                c.fetch_add(1, SeqCst);
                n.wait().unwrap();
                c.fetch_add(1, SeqCst);
                n.wait().unwrap();
                c.fetch_add(1, SeqCst);
                n.wait().unwrap();
                c.fetch_add(1, SeqCst);
            });
            t.push(h);
        }
        let to = Duration::from_millis(300);
        atomic_busywait_timeout(&ct, num_threads, to);
        notif.notify().unwrap();
        atomic_busywait_timeout(&ct, 2 * num_threads, to);
        notif.notify().unwrap();
        atomic_busywait_timeout(&ct, 3 * num_threads, to);
        notif.notify().unwrap();
        atomic_busywait_timeout(&ct, 4 * num_threads, to);
        t.into_iter().for_each(|h| h.join().unwrap());
    }

    // TODO add additional functionality tests for AosFutexNotifier
}
