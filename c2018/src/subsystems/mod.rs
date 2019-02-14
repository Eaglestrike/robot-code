pub mod controller;
pub mod drive;
pub mod superstructure;

pub trait Subsystem {
    fn run(self);
}
