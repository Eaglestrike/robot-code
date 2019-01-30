mod config;
mod subsystems;

use crate::subsystems::drive::Drive;
use bus::Bus;

fn main() {
    let bus = Bus::new(0);
    let _drive = Drive::new(bus);
    println!("Hello, world!");
}
