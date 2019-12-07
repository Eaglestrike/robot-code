use bindgen;
use cc;
use std::env;
use std::path::PathBuf;

fn main() {
    let version = bindgen::clang_version().parsed.unwrap();
    assert!(
        version.0 == 6,
        "libclang must be ^6.0, found version {}.{}",
        version.0,
        version.1
    );

    let mut build = cc::Build::new();
    build
        .file("cpp/aos/ipc_lib/aos_sync.cc")
        .file("cpp/aos/libc/aos_strerror.cc")
        .include("cpp")
        .cpp(true)
        .flag("-std=c++14")
        .warnings_into_errors(true)
        .warnings(true)
        .flag("-Wno-unused-variable")
        // .warnings_extra(true)
        .static_flag(true)
        .cargo_metadata(true);

    // if cfg!(feature = "thinlto") {
    //     build.flag("-flto=thin");
    // }
    build.compile("aos_sync");

    println!("cargo:rerun-if-changed=cpp/aos/ipc_lib/aos_sync.h");
    println!("cargo:rerun-if-changed=cpp/aos/ipc_lib/aos_sync.cc");

    let bindings = bindgen::Builder::default()
        .header("cpp/aos/ipc_lib/aos_sync.h")
        .clang_arg("-x")
        .clang_arg("c++")
        .clang_arg("-std=c++14")
        .whitelist_type("aos_.*")
        .whitelist_function("mutex_.*")
        .whitelist_function("futex_.*")
        .whitelist_function("condition_.*")
        .whitelist_recursively(false)
        .impl_debug(true)
        // TODO investigate why bindgen wont derive Debug on aos_mutex
        // It's manual impl leaves out some fields
        // This only became a problem after whitelist_recursively became false
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
