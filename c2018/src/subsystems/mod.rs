pub mod drive;

pub trait Subsystem {
    fn run(self);
}
