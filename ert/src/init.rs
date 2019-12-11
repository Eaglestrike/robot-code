use crate::{die_with_errno, panic_with_errno};
use libc::*;
use std::mem::MaybeUninit;
use std::thread;

// TODO put Rlimit setting into a std::Once
// And put it into the RT thread spawning routines
// Maybe.

// Only settable as root if we raise the hard limit
// TODO investigate how aos runs as root
fn set_soft_rlimit(resource: c_uint, soft: rlim64_t) {
    // TODO investigate why aos checks for root in their equivalent method
    let mut rlim = MaybeUninit::uninit();
    if unsafe { getrlimit64(resource, rlim.as_mut_ptr()) } == -1 {
        die_with_errno!(
            "Thread {:?} init: getrlimit({}) failed",
            thread::current().name(),
            resource
        );
    }
    let mut rlim = unsafe { rlim.assume_init() };
    rlim.rlim_cur = soft;
    rlim.rlim_max = rlim.rlim_max.max(soft);
    if unsafe { setrlimit64(resource, &rlim) } == -1 {
        die_with_errno!(
            "Thread {:?} init: getrlimit({}) failed",
            thread::current().name(),
            resource
        );
    }
}

pub fn init() {
    init_common_idempotent();
}

/// Process-level common init
fn init_common_idempotent() {
    write_core_dumps();
    // TODO init logging here
}

fn write_core_dumps() {
    // Do create core files of unlimited size.
    set_soft_rlimit(RLIMIT_CORE, RLIM_INFINITY);
}

/// Ideally called after all heap allocations complete
/// Before the robot begins to run
fn lock_all_mem() {
    set_soft_rlimit(RLIMIT_MEMLOCK, RLIM_INFINITY);

    init_common_idempotent();

    if unsafe { mlockall(MCL_CURRENT | MCL_FUTURE) } == -1 {
        die_with_errno!("mlockall failed");
    }

    // TODO replace with logger-based implementations?
    // Don't give freed memory back to the OS.
    if unsafe { mallopt(M_TRIM_THRESHOLD, -1) } != 1 {
        die!("mallopt(M_TRIM_THRESHOLD, -1) failed");
    }
    // Don't use mmap for large malloc chunks.
    if unsafe { mallopt(M_MMAP_MAX, 0) } != 1 {
        die!("mallopt(M_MMAP_MAX, 0) failed");
    }

    // Force load stack pages for main thread
    let mut stack = [1u8; 32 * 1024];
    stack.iter_mut().for_each(|byte| *byte = 2);

    // Preload some extra heap space
    let mut heap = vec![1u8; 512 * 1024];
    heap.iter_mut().for_each(|byte| *byte = 2);
}

fn set_current_thread_realtime_priority(priority: c_int) {
    // Only let rt processes run for 3 seconds straight.
    // Stop a runaway real-time process from locking up the system
    set_soft_rlimit(RLIMIT_RTTIME, 3000000);

    let param = sched_param {
        sched_priority: priority,
    };

    // TODO use log-panic
    if unsafe { sched_setscheduler(0, SCHED_FIFO, &param) } == -1 {
        panic_with_errno!("sched_setscheduler(0, SCHED_FIFO, {}) failed", priority)
    }
}

// TODO: test this with sched_getcpu ?
pub fn pin_current_thread(cpu_number: usize) {
    let mut cpuset = unsafe { std::mem::zeroed() };
    unsafe {
        CPU_ZERO(&mut cpuset);
        CPU_SET(cpu_number, &mut cpuset);
        let ret = pthread_setaffinity_np(pthread_self(), std::mem::size_of_val(&cpuset), &cpuset);
        if ret != 0 {
            die!("pthread_setaffinity_np, errno: {} {}", ret, nix::errno::Errno::from_i32(ret));
        }
    }
}

pub fn get_nprocs() -> c_long {
    unsafe { sysconf(_SC_NPROCESSORS_ONLN) }
}

#[cfg(test)]
mod isolated_tests {
    use super::*;
    use crate::test_prelude::*;
    use crate::thread::*;

    fn recurse() -> usize {
        if 2 + 2 == 4 {
            return 1 + recurse();
        }
        return 0;
    }

    rusty_fork_test! {
        #![rusty_fork(timeout_ms = 500)]
        #[cfg_attr(not(feature = "privelaged_tests"), ignore)]
        #[test]
        fn threads_inherit_priority() {
            const PRIORITY: c_int = 35;
            fn check_rt_priority() {
                // Now verify the change in thread priority
                let mut policy = MaybeUninit::uninit();
                let mut params = MaybeUninit::uninit();
                unsafe {
                    let this_thread = pthread_self();
                    let ret =
                        pthread_getschedparam(this_thread, policy.as_mut_ptr(), params.as_mut_ptr());
                    assert_eq!(ret, 0);
                    let policy = policy.assume_init();
                    let params = params.assume_init();
                    assert_eq!(policy, SCHED_FIFO);
                    assert_eq!(params.sched_priority, PRIORITY);
                }
            }
            rt_init_idempotent();
            set_current_thread_realtime_priority(PRIORITY);
            check_rt_priority();
            thread::spawn(move || {
                check_rt_priority();
            })
            .join()
            .unwrap();
        }

        #[test]
        #[should_panic]
        fn main_thread_has_stack_guard() {
            println!("{}", recurse());
        }

        #[test]
        #[should_panic]
        fn spawned_threads_have_stack_guard() {
            thread("overflow").spawn(move || {
                println!("{}", recurse());
            }).unwrap().join().unwrap();
        }
    }
}

/// Init everything that's not RT related
/// `thread_go_rt` after
pub fn thread_init_nrt() {}

pub fn thread_init_rt(relative_priority: c_int) {
    thread_init_nrt();
    thread_go_rt(relative_priority);
}

use std::sync::Once;
static RT_INIT: Once = Once::new();

fn rt_init_idempotent() {
    RT_INIT.call_once(|| {
        // Only let rt processes run for 3 seconds straight.
        // Stop a runaway real-time process from locking up the system
        set_soft_rlimit(RLIMIT_RTTIME, 3000000);

        // Allow rt processes up to priority 40.
        set_soft_rlimit(RLIMIT_RTPRIO, 40);

        lock_all_mem();
    });
}

pub fn thread_go_rt(relative_priority: c_int) {
    rt_init_idempotent();
    set_current_thread_realtime_priority(30 + relative_priority);
}

// TODO add logging to all this
// TODO test rusage for mem faults when using this interface
