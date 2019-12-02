extern crate version_check as rustc;

fn main() {
    match rustc::is_feature_flaggable() {
        Some(true) => println!("cargo:rustc-cfg=feature_flaggable"),
        _ => (),
    };
}
