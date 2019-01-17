pub mod dashboard;

pub trait Subsystem {
    fn run(&mut self);
}
