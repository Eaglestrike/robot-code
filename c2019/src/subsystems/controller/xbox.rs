use super::{get_axis, get_button, get_pov};
use wpilib::ds::*;

pub struct Xbox<'a> {
    ds: &'a DriverStation<'a>,
    port: JoystickPort,
    left_x: JoystickAxis,
    left_y: JoystickAxis,
    right_x: JoystickAxis,
    right_y: JoystickAxis,
    arrow_pad: JoystickPOV,
}

impl<'a> Xbox<'a> {
    pub fn new_from_channel(channel: u8, ds: &'a DriverStation<'a>) -> Result<Self, JoystickError> {
        Self::new_from_port(JoystickPort::new(channel)?, ds)
    }

    pub fn new_from_port(
        port: JoystickPort,
        ds: &'a DriverStation<'a>,
    ) -> Result<Self, JoystickError> {
        Ok(Self {
            ds,
            port,
            left_x: JoystickAxis::new(0)?,
            left_y: JoystickAxis::new(1)?,
            right_x: JoystickAxis::new(4)?,
            right_y: JoystickAxis::new(5)?,
            arrow_pad: JoystickPOV::new(0)?,
        })
    }

    pub fn a(&self) -> bool {
        get_button(self.ds, self.port, 1)
    }

    pub fn b(&self) -> bool {
        get_button(self.ds, self.port, 2)
    }
    pub fn x(&self) -> bool {
        get_button(self.ds, self.port, 3)
    }

    pub fn y(&self) -> bool {
        get_button(self.ds, self.port, 4)
    }

    pub fn back(&self) -> bool {
        get_button(self.ds, self.port, 7)
    }

    pub fn start(&self) -> bool {
        get_button(self.ds, self.port, 8)
    }
    pub fn left_bumper(&self) -> bool {
        get_button(self.ds, self.port, 5)
    }
    pub fn right_button(&self) -> bool {
        get_button(self.ds, self.port, 6)
    }
    pub fn left_stick_pressed(&self) -> bool {
        get_button(self.ds, self.port, 9)
    }
    pub fn right_stick_pressed(&self) -> bool {
        get_button(self.ds, self.port, 10)
    }
    pub fn left_x(&self) -> f32 {
        get_axis(self.ds, self.port, self.left_x)
    }
    pub fn left_y(&self) -> f32 {
        get_axis(self.ds, self.port, self.left_y)
    }
    pub fn right_x(&self) -> f32 {
        get_axis(self.ds, self.port, self.right_x)
    }
    pub fn right_y(&self) -> f32 {
        get_axis(self.ds, self.port, self.right_y)
    }
    pub fn arrow_pad(&self) -> i16 {
        get_pov(self.ds, self.port, self.arrow_pad)
    }
}
