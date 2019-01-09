use crate::util::config::pneumatics;
use wpilib::AnalogInput;

pub struct PressureSensor {
    pressure_sensor: AnalogInput,
}

impl PressureSensor {
    pub fn get_pressure(&self) -> f64 {
        const VCC: f64 = pneumatics::TYPICAL_PNEUMATIC_SUPPLY_VOLTAGE;
        let voltage = self.pressure_sensor.voltage().unwrap();
        250.0 * (voltage / VCC) - 25.0
    }
}
