#pragma once

#include <string>

#include <units/units.h>
#include "shims/minimal_phoenix.h"

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
    double current_limit;
    double ticks_per_inch;
    double kP;
    double kI;
    double kD;
    double ticks_per_color_slice;
    double rot_control_ticks;
    double scoot_cmd;
    int solenoid_channel;
    std::string sdb_key;
};

struct IntakeConfig {
    int rot_talon_id;
    int roller_talon_id;
    double intake_cmd;
    double rot_current_limit;
    double roller_current_limit;
    double zeroing_kp;
    double zeroing_vel;
    double abs_enc_tick_offset;
    double abs_ticks_per_rot;
    double rel_ticks_per_abs_tick;
    double rads_per_rel_tick;
    double zeroed_rad_from_vertical;
    double SinekF;
    double kP;
    double kI;
    double kD;
    double profile_vel;
    double profile_acc;
    int stowed_pos_ticks;
    int intaking_pos_ticks;
    // int trench_driving_pos_ticks;
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
    double kI;
    double kD;
    int ctre_curve_smoothing;
};

struct ShooterConifg {
    int master_id;
    int slave_id;
    double shooter_current_limit;
    double kF;
    double kP;
    double kI;
    double kD;
    VelocityMeasPeriod meas_period;
    int meas_filter_width;
    double shootable_err_pct;
    int kicker_id;
    double kicker_current_limit;
    double kicker_cmd;
};

struct BallChannelConfig {
    int serializer_id;
    int channel_id;
    double current_limit;
    double serializer_cmd;
    double channel_cmd;
    int s0_port;
    int s1_port;
    int s2_port;
    int s3_port;
};

struct ClimberConfig {
    int master_id;
    int slave_id;
    double current_limit;
    // spool winding makes this weird
    double avg_ticks_per_inch;
    int release_solenoid_id;
    int brake_solenoid_id;
    int initial_step_ticks;
    int forward_soft_limit_ticks;
    double ascend_command;
    double descend_command;
};

struct LimelightConfig {
    std::string name;
    std::string table_name;
    units::meter_t diff_height;
    units::radian_t angle_above_horizontal;
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
    LimelightConfig limelight;
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

void SetFramePeriodsForSlaveTalon(TalonSRX& talon);

}  // namespace conf
}  // namespace c2020
}  // namespace team114
