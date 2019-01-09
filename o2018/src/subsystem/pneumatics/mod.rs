use self::pressure_sensor::PressureSensor;
use crate::util::config::pneumatics::{DEFAULT_ACTIVATION_PRESSURE, DEFAULT_PRESSURE_MARGIN};
use wpilib::pneumatics::Compressor;

mod pressure_sensor;

pub struct Pneumatics {
    pressure_margin: f64,
    activation_pressure: f64,
    compressor: Compressor,
    pressure_sensor: PressureSensor,
}

impl Pneumatics {
    pub fn new(compressor: Compressor, pressure_sensor: PressureSensor) -> Self {
        Self {
            pressure_margin: DEFAULT_PRESSURE_MARGIN,
            activation_pressure: DEFAULT_ACTIVATION_PRESSURE,
            compressor,
            pressure_sensor,
        }
    }
}
