pub mod drive;
pub mod controller;

pub trait Subsystem {
    fn run(self);
}
