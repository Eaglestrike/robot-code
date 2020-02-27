#pragma once

#include <string>

#include <ctre/Phoenix.h>
#include <units/units.h>

namespace team114 {
namespace c2020 {
namespace conf {

struct DriveConfig {
    int left_master_id;
    int left_slave_id;
    int right_master_id;
    int right_slave_id;
    units::meter_t track_width;
    units::meter_t meters_per_falcon_tick;
    units::meters_per_second_t traj_max_vel;
    units::meters_per_second_squared_t traj_max_accel;
    units::meters_per_second_squared_t traj_max_centrip_accel;
};

struct ControlPanelConfig {
    int talon_id;
    int solenoid_channel;
    double ticks_per_inch;
    double kP;
    double kI;
    double kD;
};

struct IntakeConfig {
    int rot_talon_id;
    int roller_talon_id;
    double intake_cmd;
    double current_limit;
    double zeroing_kp;
    double zeroing_vel;
    double SinekF;
    double kP;
    double kI;
    double kD;
    int stowed_pos_ticks;
    int low_intaking_pos_ticks;
    int high_intaking_pos_ticks;
    int trench_driving_pos_ticks;
};

struct HoodConfig {
    int talon_id;
    double ticks_per_degree;
    double min_degrees;
    double max_degrees;
    double current_limit;
    double zeroing_current;
    double zeroing_kp;
    double zeroing_vel;
    double profile_acc;
    double profile_vel;
    double kP;
    double kD;
    int ctre_curve_smoothing;
};

struct ShooterConifg {
    int master_id;
    int slave_id;
    double current_limit;
    double kV;
    double kP;
    double kI;
    double kD;
    // TODO vel parameters n stuff
    int kicker_id;
};

struct BallChannelConfig {
    int serializer_id;
    int channel_id;
};

struct ClimberConfig {
    int master_id;
    int slave_id;
    double current_limit;
    double kP;
    double kD;
    int release_solenoid_id;
    int brake_solenoid_id;
    int initial_step_ticks;
    double windup_command;
    double winddown_command;
};

struct RobotConfig {
    std::string mac_address;
    DriveConfig drive;
    ControlPanelConfig ctrl_panel;
    IntakeConfig intake;
    HoodConfig hood;
    ShooterConifg shooter;
    BallChannelConfig ball_channel;
    ClimberConfig climber;
};

RobotConfig& GetConfig();

void DriveFalconCommonConfig(TalonFX& falcon);

void SetDriveMasterFramePeriods(TalonFX& falcon);

void SetDriveSlaveFramePeriods(TalonFX& falcon);

enum class FeedbackType {
    Quadrature,
    PulseWidth,
    Both,
    None,
};

void SetFramePeriodsForPidTalon(
    TalonSRX& talon, FeedbackType feedback_type = FeedbackType::None);

void SetFramePeriodsForOpenLoopTalon(TalonSRX& talon);

}  // namespace conf
}  // namespace c2020
}  // namespace team114
