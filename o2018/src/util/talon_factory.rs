use crate::util::config::drive::talon;
use ctre::motor_control::*;

pub trait Factory {
    fn create(id: i32) -> TalonSRX;
}

pub trait Reset {
    fn reset(&mut self);
}

pub mod motor_type {
    pub struct Ctre775Pro;

    pub struct CtreCim;

    pub struct FactoryReset;
}

impl Factory for motor_type::FactoryReset {
    fn create(id: i32) -> TalonSRX {
        let mut talon = TalonSRX::new(id);
        talon.reset();
        talon
    }
}

impl Factory for motor_type::CtreCim {
    fn create(id: i32) -> TalonSRX {
        let talon: TalonSRX = motor_type::FactoryReset::create(id);
        apply_common_config(&talon);

        talon.config_selected_feedback_sensor(
            FeedbackDevice::CTRE_MagEncoder_Relative,
            0,
            talon::TALON_CONFIG_TIMEOUT_MS,
        );
        talon.set_selected_sensor_position(0, 0, talon::TALON_CONFIG_TIMEOUT_MS);
        talon.set_quadrature_position(0, talon::TALON_CONFIG_TIMEOUT_MS);

        talon.config_peak_current_limit(
            talon::CURRENT_LIMIT_THRESHOLD,
            talon::TALON_CONFIG_TIMEOUT_MS,
        );
        talon.config_peak_current_duration(
            talon::CURRENT_LIMIT_DURATION_MS,
            talon::TALON_CONFIG_TIMEOUT_MS,
        );
        talon.config_continuous_current_limit(talon::CURRENT_LIMIT, talon::TALON_CONFIG_TIMEOUT_MS);
        talon.enable_current_limit(true);

        talon.config_nominal_output_forward(0.0, 10);
        talon.config_nominal_output_reverse(0.0, 10);
        talon.config_peak_output_forward(1.0, 10);
        talon.config_peak_output_reverse(-1.0, 10);
        talon
    }
}

impl Factory for motor_type::Ctre775Pro {
    fn create(id: i32) -> TalonSRX {
        let talon: TalonSRX = motor_type::FactoryReset::create(id);
        apply_common_config(&talon);

        talon.config_peak_current_limit(
            talon::CURRENT_LIMIT_THRESHOLD,
            talon::TALON_CONFIG_TIMEOUT_MS,
        );
        talon.config_peak_current_duration(
            talon::CURRENT_LIMIT_DURATION_MS,
            talon::TALON_CONFIG_TIMEOUT_MS,
        );
        talon.config_continuous_current_limit(talon::CURRENT_LIMIT, talon::TALON_CONFIG_TIMEOUT_MS);
        talon.enable_current_limit(true);
        talon
    }
}

fn apply_common_config(talon: &TalonSRX) {
    talon.set_inverted(false);
    talon.set_sensor_phase(false);
    talon.set_neutral_mode(NeutralMode::Brake);
}

// Based on https://github.com/CrossTheRoadElec/Phoenix-Documentation#factory-default-values
// This has the same effect as pressing the reset button on the talon.
impl Reset for TalonSRX {
    fn reset(&mut self) {
        self.config_openloop_ramp(0.0, 0);
        self.config_closedloop_ramp(0.0, 0);
        self.config_peak_output_forward(1.0, 0);
        self.config_peak_output_reverse(-1.0, 0);
        self.config_nominal_output_forward(0.0, 0);
        self.config_nominal_output_reverse(0.0, 0);
        self.config_neutral_deadband(0.04, 0);
        self.config_voltage_comp_saturation(0.0, 0);
        self.config_voltage_measurement_filter(32, 0);
        self.config_selected_feedback_sensor(FeedbackDevice::None, 0, 0);

        self.config_remote_feedback_filter(0, RemoteSensorSource::Off, 0, 0);
        self.config_sensor_term(SensorTerm::Diff0, FeedbackDevice::None, 0);
        self.config_sensor_term(SensorTerm::Diff1, FeedbackDevice::None, 0);
        self.config_sensor_term(SensorTerm::Sum0, FeedbackDevice::None, 0);
        self.config_sensor_term(SensorTerm::Sum1, FeedbackDevice::None, 0);

        self.config_velocity_measurement_period(VelocityMeasPeriod::Period_100Ms, 0);
        self.config_velocity_measurement_window(64, 0);
        self.config_forward_limit_switch_source(
            LimitSwitchSource::Deactivated,
            LimitSwitchNormal::NormallyOpen,
            0,
        );
        self.config_reverse_limit_switch_source(
            LimitSwitchSource::Deactivated,
            LimitSwitchNormal::NormallyOpen,
            0,
        );

        self.config_forward_soft_limit_threshold(0, 0);
        self.config_reverse_soft_limit_threshold(0, 0);
        self.config_forward_soft_limit_enable(false, 0);
        self.config_reverse_soft_limit_enable(false, 0);

        self.config_kp(0, 0.0, 0);
        self.config_ki(0, 0.0, 0);
        self.config_kd(0, 0.0, 0);
        self.config_kf(0, 0.0, 0);
        self.config_integral_zone(0, 0, 0);
        self.config_allowable_closedloop_error(0, 0, 0);
        self.config_max_integral_accumulator(0, 0.0, 0);

        self.config_motion_cruise_velocity(0, 0);
        self.config_motion_acceleration(0, 0);
        self.config_motion_profile_trajectory_period(0, 0);
        self.config_set_custom_param(0, 0, 0);
        self.config_peak_current_limit(0, 0);
        self.config_peak_current_duration(0, 0);
        self.config_continuous_current_limit(0, 0);
    }
}
