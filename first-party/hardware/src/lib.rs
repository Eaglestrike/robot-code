use wpilib::{AnalogInput, HalResult};

#[derive(Debug)]
pub struct Rev111107DS00PressureSensor(AnalogInput, f64);

impl Rev111107DS00PressureSensor {
    pub fn new(ain: AnalogInput, supply_voltage: f64) -> Self {
        Self(ain, supply_voltage)
    }

    pub fn psi(&self) -> HalResult<f64> {
        self.0.voltage().map(|v| (250.0 * v / self.1) - 25.0)
    }

    pub fn into_inner(self) -> AnalogInput {
        self.0
    }
}

impl From<AnalogInput> for Rev111107DS00PressureSensor {
    fn from(a: AnalogInput) -> Self {
        Self::new(a, 12.0)
    }
}
