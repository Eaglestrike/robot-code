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
    double orient_kp;
    double orient_ki;
    double orient_kd;
    double orient_vel;
    double orient_acc;
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
    int master_id; /**< Each motor has an ID. There are 2 shooter motors performing the same action, and this is the master ID. */
    int slave_id; /**< The respective slave ID. */
    double shooter_current_limit; /**< The current limit (in amps) of the talons */
    double kF; /**< F (feedforward) term in shooter PID */
    double kP; /**< P term in shooter PID */
    double kI; /**< I term in shooter PID */
    double kD; /**< D term in shooter PID */
    VelocityMeasPeriod meas_period; /**< Period of time over which velocidty is measured */
    int meas_filter_width; /**< Not sure, is there some sort of filter on the shooter? */
    double shootable_err_pct; /**< Perhaps ammount of error acceptable when trying to make a shot? */
    int kicker_id; /**< Perhaps another talon motor that could be enabled to help the shooter wheel spin faster? */
    double kicker_current_limit; /**< Current limit (amps) for kicker talon */
    double kicker_cmd; /**< Percent output for kicker talon when engaged */
};

struct BallChannelConfig {
    int serializer_id; /**< Talon ID of serializing(?) talon motor **/
    int channel_id; /**< Talon ID of another talon motor, perhaps to help channel the balls? **/
    double current_limit; /**< The current limit (amps) of the talons **/
    double serializer_cmd; /**< Percent output to serializer talon for it to serialize? **/
    double channel_cmd; /**< Percent output to channel motor for it to channel? **/
    int s0_port; /**< I'm guessing this would be one of 4 sensors in the serializer, perhaps to gain info on state of balls. **/
    int s1_port; /**< I'm guessing this would be one of 4 sensors in the serializer, perhaps to gain info on state of balls. **/
    int s2_port; /**< I'm guessing this would be one of 4 sensors in the serializer, perhaps to gain info on state of balls. **/
    int s3_port; /**< I'm guessing this would be one of 4 sensors in the serializer, perhaps to gain info on state of balls. **/
};

struct ClimberConfig {
    int master_id; /**< ID of master talon motor **/
    int slave_id; /**< ID of slave talon motor **/
    double current_limit; /**< The current limit (amps) of the talons **/
    // spool winding makes this weird
    double avg_ticks_per_inch; /**< Probably number of ticks on motors that pass for climber to raise one inch. **/
    int release_solenoid_id; /**< ID of solenoid that controls the release pneumatic (to raise/lower climber) **/
    int brake_solenoid_id; /**< ID of brake solenoid, I think it's purpose is to hold the climber in position? **/
    int initial_step_ticks; /**< Probably the amount of ticks the motors should turn initially to raise the climber. **/
    int forward_soft_limit_ticks; /**< Probably a soft limit for how many ticks the motors should turn. **/
    double ascend_command; /**< Percent output to talons to raise climber (positive value) **/
    double descend_command; /**< Percent output to talons to lower climber (negative value) **/
};

struct LimelightConfig {
    std::string name; /**< Unused? Name of limelight**/
    std::string table_name; /**< Name used for limelight NetworkTable**/
    units::meter_t diff_height; /**< Not sure, unused **/
    units::radian_t angle_above_horizontal; /**< Probaly offset, camera won't be exactly level **/
};

struct RobotConfig {
    std::string mac_address; /**< For identifying roboRIO **/
    DriveConfig drive; /**< For the drive base, go to DriveConfig for more info **/
    ControlPanelConfig ctrl_panel; /**< For control panel, go to ControlPanelConfig for more info **/
    IntakeConfig intake; /**< For the intake, go to IntakeConfig for more info **/
    HoodConfig hood; /**< For the shooter hood, go to HoodConfig for more info **/
    ShooterConifg shooter; /**< Perhaps scrapped, but for entire shooter. Go to ShooterConifg for more info **/
    BallChannelConfig ball_channel; /**< For the ball channel, go to BallChannelConfig for more info **/
    ClimberConfig climber; /**< For the climber, go to ClimberConfig for more info **/
    LimelightConfig limelight; /**< For the limelight, go to LimelightConfig for more info **/
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
