mod config;
mod subsystems;

use crate::subsystems::drive::Drive;
use bus::Bus;
use crossbeam_channel::unbounded;

fn main() {
    let bus = Bus::new(0);
    let (_, recv) = unbounded();
    let _drive = Drive::new(bus, recv);
    println!("Hello, world!");
}
