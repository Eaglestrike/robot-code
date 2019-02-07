pub mod controller;
pub mod drive;
mod superstructure;

pub trait Subsystem {
    fn run(self);
}
