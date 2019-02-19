pub mod edge_detect;
#[allow(dead_code)]
pub mod xbox;

use super::{
    drive::{Gear, Instruction as DriveCmd},
    superstructure, Subsystem,
};
use crate::cheesy_drive::CheesyDrive;
use crossbeam_channel::Sender;
use superstructure::{HatchPneumaticExt, Instruction as SsCmd, UserElevatorHeights};
use wpilib::ds::*;

type Drive = Sender<DriveCmd>;
type Superstructure = Sender<SsCmd>;

#[allow(dead_code)]
#[derive(Debug)]
pub struct Controller<'a, T: Controls> {
    controls: EdgeWrapper<T>,
    cheesy: CheesyDrive,
    drive: Drive,
    superstructure: Superstructure,
    ds: DriverStation<'a>,
}

impl<'a, T: Controls> Controller<'a, T> {
    pub fn new(
        controls: T,
        drive: Drive,
        superstructure: Superstructure,
        ds: DriverStation<'a>,
    ) -> Self {
        Self {
            controls: EdgeWrapper::new(controls),
            cheesy: CheesyDrive::new(),
            drive,
            superstructure,
            ds,
        }
    }
}

impl<'a, T: Controls> Subsystem for Controller<'a, T> {
    fn run(mut self) {
        loop {
            self.ds.wait_for_data();
            match self.ds.robot_state() {
                RobotState::Disabled => {
                    continue;
                }
                _ => {}
            }

            // TODO add logging to all this

            // DRIVE
            let wheel = self.controls.wheel();
            let throttle = self.controls.throttle();
            let quick_turn = self.controls.quick_turn_raw();
            let high_gear = !self.controls.low_gear_raw();
            // TODO user input
            let signal = self
                .cheesy
                .cheesy_drive(wheel, throttle, quick_turn, high_gear);
            self.drive
                .send(DriveCmd::Percentage(signal.l, signal.r))
                .expect("Channel disconnected: ");
            // TODO log
            self.controls.low_gear().sig_send_val(
                DriveCmd::GearShift(Gear::Low),
                DriveCmd::GearShift(Gear::High),
                |cmd| {
                    self.drive.send(cmd).expect("DT disconnected");
                },
            );

            // SUPERSTRUCTURE

            // TODO log
            if self.controls.ball_intake().rising() {
                self.superstructure
                    .send(SsCmd::BallIntake)
                    .expect("SS disconnected");
            }
            if self.controls.abort_ball_intake().rising() {
                self.superstructure
                    .send(SsCmd::AbortIntake)
                    .expect("SS disconnected");
            }
            if self.controls.outtake_ball().rising() {
                self.superstructure
                    .send(SsCmd::BallOuttake)
                    .expect("SS disconnected");
            }
            self.controls.ball_unjam().sig_send(|is_unjam| {
                self.superstructure
                    .send(SsCmd::Unjam(is_unjam))
                    .expect("SS disconnected");
            });
            self.controls.hatch_extend().sig_send_val(
                SsCmd::HatchExtend(HatchPneumaticExt::Extended),
                SsCmd::HatchExtend(HatchPneumaticExt::Retracted),
                |cmd| {
                    self.superstructure.send(cmd).expect("SS disconnected");
                },
            );
            self.controls.hatch_outtake().sig_send_val(
                SsCmd::HatchOuttake(HatchPneumaticExt::Extended),
                SsCmd::HatchOuttake(HatchPneumaticExt::Retracted),
                |cmd| {
                    self.superstructure.send(cmd).expect("SS disconnected");
                },
            );
            if self.controls.elevator_low().rising() {
                self.superstructure
                    .send(SsCmd::SetElevatorHeight(UserElevatorHeights::Low))
                    .expect("SS disconnected");
            }
            if self.controls.elevator_med().rising() {
                self.superstructure
                    .send(SsCmd::SetElevatorHeight(UserElevatorHeights::Med))
                    .expect("SS disconnected");
            }
            if self.controls.elevator_high().rising() {
                self.superstructure
                    .send(SsCmd::SetElevatorHeight(
                        superstructure::UserElevatorHeights::High,
                    ))
                    .expect("SS disconnected");
            }
            if self.controls.elevator_cargo().rising() {
                self.superstructure
                    .send(SsCmd::SetElevatorHeight(
                        superstructure::UserElevatorHeights::Cargo,
                    ))
                    .expect("SS disconnected");
            }
        }
    }
}

extern crate paste;
macro_rules! wrapper_fields {
    ($name:ident, $( $x:ident ),*) => {
        #[derive(Debug)]
        pub struct $name<T: Controls> {
            c: T,
        $(
            $x: EdgeDetector,
        )*
        }

        impl<T: Controls> $name<T> {
            pub fn new(c: T) -> Self {
                Self {
                    c,
                    $(
                        $x: EdgeDetector::new(),
                    )*
                }
            }

            $(
                pub fn $x (&mut self) -> Edge {
                    self.$x.get(self.c.$x())
                }
            )*

            paste::item! {
                $(
                    pub fn [<$x _raw>] (&mut self) -> bool {
                        self.c.$x()
                    }
                )*
            }
        }
    };
}

#[allow(dead_code)]
mod _wrapper {
    use super::{
        edge_detect::{Edge, EdgeDetector},
        Controls,
    };
    wrapper_fields! { EdgeWrapper,
        low_gear,
        quick_turn,
        ball_intake,
        abort_ball_intake,
        outtake_ball,
        ball_unjam,
        hatch_extend,
        hatch_outtake,
        elevator_low,
        elevator_med,
        elevator_high,
        elevator_cargo
    }

    impl<T: Controls> EdgeWrapper<T> {
        pub fn throttle(&mut self) -> f64 {
            self.c.throttle()
        }

        pub fn wheel(&mut self) -> f64 {
            self.c.wheel()
        }
    }
}
use _wrapper::EdgeWrapper;

// Implement this for a raw controller struct
pub trait Controls {
    fn throttle(&mut self) -> f64;
    fn wheel(&mut self) -> f64;
    fn low_gear(&mut self) -> bool;
    fn quick_turn(&mut self) -> bool;
    fn ball_intake(&mut self) -> bool;
    fn abort_ball_intake(&mut self) -> bool;
    fn outtake_ball(&mut self) -> bool;
    fn ball_unjam(&mut self) -> bool;
    fn hatch_extend(&mut self) -> bool;
    fn hatch_outtake(&mut self) -> bool;
    fn elevator_low(&mut self) -> bool;
    fn elevator_med(&mut self) -> bool;
    fn elevator_high(&mut self) -> bool;
    fn elevator_cargo(&mut self) -> bool;
}

#[derive(Debug)]
pub struct StandardControls<'a> {
    ds: DriverStation<'a>,
    left: JoystickPort,
    right: JoystickPort,
    oi: JoystickPort,
    x: JoystickAxis,
    y: JoystickAxis,
}

impl<'a> StandardControls<'a> {
    pub fn new(
        ds: DriverStation<'a>,
        left: JoystickPort,
        right: JoystickPort,
        oi: JoystickPort,
    ) -> Result<Self, JoystickError> {
        Ok(Self {
            ds,
            left,
            right,
            oi,
            x: JoystickAxis::new(0)?,
            y: JoystickAxis::new(1)?,
        })
    }
}
impl<'a> Controls for StandardControls<'a> {
    fn throttle(&mut self) -> f64 {
        (-get_axis(&self.ds, self.left, self.y)).into()
    }
    fn wheel(&mut self) -> f64 {
        get_axis(&self.ds, self.right, self.x).into()
    }
    fn low_gear(&mut self) -> bool {
        get_button(&self.ds, self.left, 0)
    }
    fn quick_turn(&mut self) -> bool {
        get_button(&self.ds, self.right, 0)
    }
    //TODO: Bind these
    fn ball_intake(&mut self) -> bool {
        get_button(&self.ds, self.oi, 4)
    }
    fn abort_ball_intake(&mut self) -> bool {
        get_button(&self.ds, self.oi, 6)
    }
    fn outtake_ball(&mut self) -> bool {
        get_button(&self.ds, self.oi, 5)
    }
    fn ball_unjam(&mut self) -> bool {
        get_button(&self.ds, self.oi, 8)
    }
    fn hatch_extend(&mut self) -> bool {
        get_axis(&self.ds, self.oi, JoystickAxis::new(2).unwrap()) > 0.8
    }
    fn hatch_outtake(&mut self) -> bool {
        get_axis(&self.ds, self.oi, JoystickAxis::new(3).unwrap()) > 0.8
    }
    fn elevator_low(&mut self) -> bool {
        get_button(&self.ds, self.oi, 0)
    }
    fn elevator_med(&mut self) -> bool {
        get_button(&self.ds, self.oi, 1)
    }
    fn elevator_high(&mut self) -> bool {
        get_button(&self.ds, self.oi, 3)
    }
    fn elevator_cargo(&mut self) -> bool {
        get_button(&self.ds, self.oi, 2)
    }
}
fn get_button(ds: &DriverStation<'_>, port: JoystickPort, num: u8) -> bool {
    ds.stick_button(port, num).unwrap_or(false)
}
fn get_axis(ds: &DriverStation<'_>, port: JoystickPort, axis: JoystickAxis) -> f32 {
    ds.stick_axis(port, axis).unwrap_or(0.0)
}
fn get_pov(ds: &DriverStation<'_>, port: JoystickPort, pov: JoystickPOV) -> i16 {
    ds.stick_pov(port, pov).unwrap_or(0)
}
