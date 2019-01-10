use wpilib::AnalogInput;
use wpilib::pneumatics::Compressor;

use crate::util::config::pneumatics::*;

pub struct Pneumatics {
    pressure_margin: f64,
    activation_pressure: f64,
    compressor: Compressor,
    pressure_sensor: PressureSensor,
}

impl Pneumatics {
    pub fn new() -> Self {
        let pressure_in = AnalogInput::new(PNEUMATIC_PRESSURE_SENSOR_ID).expect("Unable to create analog input for the pneumatics sensor!");

        Self {
            pressure_margin: DEFAULT_PRESSURE_MARGIN,
            activation_pressure: DEFAULT_ACTIVATION_PRESSURE,
            compressor: Compressor::new().expect("Unable to connect to the compressor!"),
            pressure_sensor: PressureSensor(pressure_in),
        }
    }

// TODO: Aris put this in a thread and make things work!
//    fn tick(&mut self) {
//        if self.activation_pressure < 0.0 {
//            self.compressor.start();
//        } else if self.get_pressure() < self.activation_pressure
//            && !self.compressor.get_closed_loop_control()
//        {
//            self.compressor.start();
//        } else if (self.get_pressure() > (self.activation_pressure + self.pressure_margin))
//            && self.compressor.get_closed_loop_control()
//        {
//            self.compressor.stop();
//        }
//    }

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

struct PressureSensor (AnalogInput);

impl PressureSensor {
    fn get(&self) -> f64 {
        const VCC: f64 = TYPICAL_PNEUMATIC_SUPPLY_VOLTAGE;
        let voltage = self.0.voltage().expect("Unable to get data from pressure sensor!");
        250.0 * (voltage / VCC) - 25.0
    }
}
