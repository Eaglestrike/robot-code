use ctre::motor_control::*;
use ctre::ErrorCode;
use ctre::motor_control::config::*;

use crate::config::drive;

/// Extension for the talon which contains a factory reset and new function which will return a
/// configured version of the specified motor.
pub trait TalonExt {
    fn new_cim(id: i32) -> Result<TalonSRX, ErrorCode>;
    fn new_775pro(id: i32) -> Result<TalonSRX, ErrorCode>;
    fn reset(&mut self) -> Result<(), ErrorCode>;
}

impl TalonExt for TalonSRX {
    fn new_cim(id: i32) -> Result<TalonSRX, ErrorCode> {
        let mut talon: TalonSRX = TalonSRX::new(id);
        talon.reset()?;


        talon.set_inverted(false);
        talon.set_sensor_phase(false);
        talon.set_neutral_mode(NeutralMode::Brake);

        talon.config_selected_feedback_sensor(
            FeedbackDevice::CTRE_MagEncoder_Relative,
            0,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.set_selected_sensor_position(0, 0, drive::TALON_CONFIG_TIMEOUT_MS)?;
        talon.set_quadrature_position(0, drive::TALON_CONFIG_TIMEOUT_MS)?;

        talon.config_peak_current_limit(
            drive::CURRENT_LIMIT_THRESHOLD,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.config_peak_current_duration(
            drive::CURRENT_LIMIT_DURATION_MS,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.config_continuous_current_limit(
            drive::CURRENT_LIMIT,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.enable_current_limit(true);

        talon.config_nominal_output_forward(0.0, 10)?;
        talon.config_nominal_output_reverse(0.0, 10)?;
        talon.config_peak_output_forward(1.0, 10)?;
        talon.config_peak_output_reverse(-1.0, 10)?;
        Ok(talon)
    }

    fn new_775pro(id: i32) -> Result<TalonSRX, ErrorCode> {
        let mut talon: TalonSRX = TalonSRX::new(id);
        talon.reset()?;

        talon.set_inverted(false);
        talon.set_sensor_phase(false);
        talon.set_neutral_mode(NeutralMode::Brake);

        talon.config_peak_current_limit(
            drive::CURRENT_LIMIT_THRESHOLD,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.config_peak_current_duration(
            drive::CURRENT_LIMIT_DURATION_MS,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.config_continuous_current_limit(
            drive::CURRENT_LIMIT,
            drive::TALON_CONFIG_TIMEOUT_MS,
        )?;
        talon.enable_current_limit(true);
        Ok(talon)
    }

    /// Based on https://github.com/CrossTheRoadElec/Phoenix-Documentation#factory-default-values
    /// This has the same effect as pressing the reset button on the talon.
    fn reset(&mut self) -> Result<(), ErrorCode> {
        self.config_openloop_ramp(0.0, 0)?;
        self.config_closedloop_ramp(0.0, 0)?;
        self.config_peak_output_forward(1.0, 0)?;
        self.config_peak_output_reverse(-1.0, 0)?;
        self.config_nominal_output_forward(0.0, 0)?;
        self.config_nominal_output_reverse(0.0, 0)?;
        self.config_neutral_deadband(0.04, 0)?;
        self.config_voltage_comp_saturation(0.0, 0)?;
        self.config_voltage_measurement_filter(32, 0)?;
        self.config_selected_feedback_sensor(FeedbackDevice::QuadEncoder, 0, 0)?;

        self.config_remote_feedback_filter(0, RemoteSensorSource::Off, 0, 0)?;
        self.config_sensor_term(SensorTerm::Diff0, FeedbackDevice::QuadEncoder, 0)?;
        self.config_sensor_term(SensorTerm::Diff1, FeedbackDevice::QuadEncoder, 0)?;
        self.config_sensor_term(SensorTerm::Sum0, FeedbackDevice::QuadEncoder, 0)?;
        self.config_sensor_term(SensorTerm::Sum1, FeedbackDevice::QuadEncoder, 0)?;

        self.config_velocity_measurement_period(VelocityMeasPeriod::Period_100Ms, 0)?;
        self.config_velocity_measurement_window(64, 0)?;
        self.config_forward_limit_switch_source(
            LimitSwitchSource::Deactivated,
            LimitSwitchNormal::NormallyOpen,
            0,
        )?;
        self.config_reverse_limit_switch_source(
            LimitSwitchSource::Deactivated,
            LimitSwitchNormal::NormallyOpen,
            0,
        )?;

        self.config_forward_soft_limit_threshold(0, 0)?;
        self.config_reverse_soft_limit_threshold(0, 0)?;
        self.config_forward_soft_limit_enable(false, 0)?;
        self.config_reverse_soft_limit_enable(false, 0)?;

        self.config_kp(0, 0.0, 0)?;
        self.config_ki(0, 0.0, 0)?;
        self.config_kd(0, 0.0, 0)?;
        self.config_kf(0, 0.0, 0)?;
        self.config_integral_zone(0, 0, 0)?;
        self.config_allowable_closedloop_error(0, 0, 0)?;
        self.config_max_integral_accumulator(0, 0.0, 0)?;

        self.config_motion_cruise_velocity(0, 0)?;
        self.config_motion_acceleration(0, 0)?;
        self.config_motion_profile_trajectory_period(0, 0)?;
        self.config_set_custom_param(0, 0, 0)?;
        self.config_peak_current_limit(0, 0)?;
        self.config_peak_current_duration(0, 0)?;
        self.config_continuous_current_limit(0, 0)
    }
}
