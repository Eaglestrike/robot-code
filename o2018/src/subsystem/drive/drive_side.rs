use super::DualPwm;
use crate::util::config;
use ctre::motor_control::*;

pub trait DriveSide {
    fn set_inverted(&mut self, inverted: bool);
    fn set_percent(&mut self, percentage: f64);
    fn set_velocity(&mut self, feet_per_second: f64);
    fn position_ticks(&self) -> i32;
    fn position(&self) -> f64;
    fn velocity(&self) -> f64;
}

#[allow(dead_code)]
pub struct TalonDriveSide {
    master: TalonSRX,
    slave: TalonSRX, //TODO: Once set as a follower, its basically just a memory leak. Decide if we should remove it.
}

impl TalonDriveSide {
    pub fn new(master: TalonSRX, slave: TalonSRX) -> Self {
        let drive_side: TalonDriveSide = TalonDriveSide { master, slave };
        drive_side.config();
        drive_side
    }

    fn config(&self) {
        self.slave.set(
            ControlMode::Follower,
            f64::from(self.master.get_device_id()),
            DemandType::Neutral,
            0.0,
        );
        self.master
            .config_kf(config::drive_side::LOW_GEAR_VEL_PID_IDX, 0.279_714_286, 10);
        self.master.config_kp(0, 0.5, 10);
        self.master.config_ki(0, 0.0, 10);
        self.master.config_kd(0, 2.5, 10);
    }

    pub fn config_frame_period(&self, period: i32) {
        self.master
            .set_status_frame_period(StatusFrameEnhanced::Status_3_Quadrature, period, 0);
    }
}

impl DriveSide for TalonDriveSide {
    fn set_inverted(&mut self, inverted: bool) {
        self.master.set_inverted(inverted);
        self.slave.set_inverted(inverted);
    }

    fn set_percent(&mut self, percentage: f64) {
        self.master.set(
            ControlMode::PercentOutput,
            percentage,
            DemandType::Neutral,
            0.0,
        );
    }

    fn set_velocity(&mut self, feet_per_second: f64) {
        self.master.set(
            ControlMode::Velocity,
            feet_per_second * 0.1,
            DemandType::Neutral,
            0.0,
        );
    }

    fn position_ticks(&self) -> i32 {
        self.master.get_selected_sensor_position(0).unwrap()
    }

    fn position(&self) -> f64 {
        // we have to use this function to get position so that sensorPhase is taken into account
        // undocumented behavior in Phoenix
        f64::from(self.master.get_selected_sensor_position(0).unwrap())
            * config::drive::DRIVE_ENCODER_FEET_PER_TICK
    }

    fn velocity(&self) -> f64 {
        f64::from(self.master.get_selected_sensor_velocity(0).unwrap())
            * config::drive::DRIVE_ENCODER_FEET_PER_TICK
    }
}

pub struct PwmDriveSide {
    pwm: DualPwm,
    k_acceleration: f64,
    k_velocity: f64,
}

impl PwmDriveSide {
    pub fn new(pwm: DualPwm, k_acceleration: f64, k_velocity: f64) -> Self {
        PwmDriveSide {
            pwm,
            k_acceleration,
            k_velocity,
        }
    }
}

impl DriveSide for PwmDriveSide {
    fn set_inverted(&mut self, inverted: bool) {
        self.pwm.set_inverted(inverted)
    }

    fn set_percent(&mut self, percentage: f64) {
        self.pwm.set(percentage)
    }

    fn set_velocity(&mut self, feet_per_second: f64) {
        self.pwm
            .set(self.k_velocity * feet_per_second * config::drive::DRIVE_ENCODER_TICKS_PER_FOOT)
    }

    fn position_ticks(&self) -> i32 {
        self.pwm.position()
    }

    fn position(&self) -> f64 {
        self.pwm.position() as f64 * config::drive::DRIVE_ENCODER_FEET_PER_TICK
    }

    fn velocity(&self) -> f64 {
        self.pwm.rate() * config::drive::DRIVE_ENCODER_FEET_PER_TICK
    }
}
