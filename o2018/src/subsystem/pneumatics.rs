use std::thread;
use std::time::Duration;

use bus::BusReader;
use crossbeam_channel::{unbounded, Receiver, Sender};
use wpilib::pneumatics::Compressor;
use wpilib::AnalogInput;

use crate::util::config::pneumatics::*;

use super::Subsystem;

pub struct Pneumatics {
    pressure_margin: f64,
    activation_pressure: f64,
    compressor: Compressor,
    pressure_sensor: PressureSensor,
    receiver: Receiver<Instruction>,
}

impl Pneumatics {
    fn new() -> (Self, Sender<Instruction>) {
        let pressure_in = AnalogInput::new(PNEUMATIC_PRESSURE_SENSOR_ID)
            .expect("Unable to create analog input for the pneumatics sensor!");

        let (sender, receiver) = unbounded();

        let pneumatics = Pneumatics {
            pressure_margin: DEFAULT_PRESSURE_MARGIN,
            activation_pressure: DEFAULT_ACTIVATION_PRESSURE,
            compressor: Compressor::new().expect("Unable to connect to the compressor!"),
            pressure_sensor: PressureSensor(pressure_in),
            receiver,
        };

        (pneumatics, sender)
    }

    #[allow(dead_code)]
    pub fn pressure(&self) -> f64 {
        self.pressure_sensor.get()
    }

    #[allow(dead_code)]
    pub fn compressing(&self) -> bool {
        self.compressor.enabled()
    }

    pub fn set_minimum_pressure(&mut self, pressure: f64) {
        self.activation_pressure = pressure.min(100.0);
    }

    pub fn set_pressure_margin(&mut self, margin: f64) {
        self.pressure_margin = margin;
    }
}

pub enum Instruction {
    Enable,
    Disable,
    MinPressure(f64),
    PressureMargin(f64),
}

// TODO: Decide if this should be moved to the config or remain in the subsystem.
/// Update rate for compressing. Much lower than other threads since it is not as important and
/// does not react as fast as many of the other subsystems.
const UPDATE_RATE: Duration = Duration::from_millis(100);

impl Subsystem<()> for Pneumatics {
    fn run(&mut self) {
        let mut enabled = true;
        loop {
            for item in self.receiver.try_iter() {
                match item {
                    Instruction::MinPressure(val) => self.activation_pressure = val.min(100.0),
                    Instruction::PressureMargin(val) => self.pressure_margin = val,
                    Instruction::Disable => {
                        self.compressor.stop();
                        enabled = false
                    }
                    Instruction::Enable => enabled = true,
                }
            }

            if enabled {
                if self.activation_pressure < 0.0 {
                    self.compressor.start();
                } else if self.pressure() < self.activation_pressure
                    && !self.compressor.closed_loop_control()
                {
                    self.compressor.start();
                } else if self.pressure() > self.activation_pressure + self.pressure_margin
                    && self.compressor.closed_loop_control()
                {
                    self.compressor.stop();
                }
            }

            thread::sleep(UPDATE_RATE);
        }
    }

    fn create_receiver(&mut self) -> BusReader<()> {
        unimplemented!("The pneumatics system does not send any data!")
    }
}

struct PressureSensor(AnalogInput);

impl PressureSensor {
    #[inline(always)]
    fn get(&self) -> f64 {
        const VCC: f64 = TYPICAL_PNEUMATIC_SUPPLY_VOLTAGE;
        let voltage = self
            .0
            .voltage()
            .expect("Unable to get data from pressure sensor!");
        250.0 * (voltage / VCC) - 25.0
    }
}
