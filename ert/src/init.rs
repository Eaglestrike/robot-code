use libc::*;

fn set_soft_rlimit(resource: c_int, soft: rlimit64_t) {}
