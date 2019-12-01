use libc::*;

fn set_soft_rlimit(_resource: c_int, _soft: rlim64_t) {}
