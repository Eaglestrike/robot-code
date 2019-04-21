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

// TODO add Chief Delphi link for the circuit fix
/// QMIC 0N 0A Retroreflective Infrared NPN Proximity Sensor
///
/// # Notes
/// This sensor sucks for the RIO. Avoid whenever possible.
///
/// Like many standard sensors, it runs of 15V.
/// Advice: Use boost converters on the 5v DIO rail.
///
/// It doesn't sink enough current to bring down the pulled-up DIO signal
/// line voltage. The solution is a resistor-based circuit that brings down
/// the low voltage while keeping the high voltage high enough.
pub type Qmic0N0A = wpilib::dio::DigitalInput;
