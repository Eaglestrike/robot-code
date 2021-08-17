#pragma once

#include <string>

#include <units/units.h>
#include "shims/minimal_phoenix.h"

namespace team114 {
namespace c2020 {
namespace conf {

struct DriveConfig {
    int left_master_id; /** < talon id of master drive motor on left side **/
    int left_slave_id;  /** < talon id of slave drive motor on left side **/
    int right_master_id; /** < talon id of master drive motor on right side **/
    int right_slave_id; /** < talon id of slave drive motor on right side **/
    units::meter_t track_width;
    units::meter_t meters_per_falcon_tick; /**< self explanitory, look up what a frc falcon tick is **/
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
    int talon_id; /** < talon id for control panel motor **/
    double current_limit; /** < current limit (amps) for said motor **/
    double ticks_per_inch; /**< self explanitory, look up what a frc falcon tick is **/
    double kP; /**< P constant for this motor's pid **/
    double kI; /**< I constant for this motor's pid **/
    double kD; /**< D constant for this motor's pid **/
    double ticks_per_color_slice; /**< # of ticks to rotate the color wheel one "slice" (which is a specific angle) **/
    double rot_control_ticks;
    double scoot_cmd;
    int solenoid_channel; /**< I think the wheel is lifted with a pneumatic, and this is likely its solenoid **/
    std::string sdb_key;
};

struct IntakeConfig {
    int rot_talon_id;
    int roller_talon_id;
    double intake_cmd; /**< percent output for when intake is running **/
    double rot_current_limit; /**< current limit (amps) for rotator motor **/
    double roller_current_limit; /**< current limit (amps) for roller motor **/
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
    int talon_id; /**< talon id of motor that moves the shooter hood **/
    double ticks_per_degree; /**< self explanitory, look up what a frc falcon tick is **/
    double min_degrees; /** < smallest angle hood can move to **/
    double max_degrees; /** < largest angle hood can move to **/
    double current_limit; /** < current limit (amps) for hood motor **/
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
    int master_id; /**< The shooter flywheel takes 2 motors to spin, this is the master's id. left of shooter **/
    int slave_id; /**< The respective slave ID. Right of shooter. **/
    double shooter_current_limit; /**< The current limit (amps) of the talons **/
    double kF; /**< F (feedforward) term in shooter PID **/
    double kP; /**< P term in shooter PID **/
    double kI; /**< I term in shooter PID **/
    double kD; /**< D term in shooter PID **/
    VelocityMeasPeriod meas_period; /**< Talons can return "velocity". Velocity is measured every period and averaged. */
    int meas_filter_width; /**< Helps with velocity measurement. **/
    double shootable_err_pct; /**< Perhaps ammount of error acceptable when trying to make a shot? idk josh wrote this and we now have a new system*/
    int kicker_id; /**< Motor/wheel that spins weels into shooter wheel. **/
    double kicker_current_limit; /**< Current limit (amps) for kicker talon (small green wheel below flywheel, helps feed balls into shooter) */
    double kicker_cmd; /**< Percent output for kicker talon when engaged **/
};

struct BallChannelConfig {
    int serializer_id; /**< Talon ID of serializing talon motor. Serializer is in front of robot, it feeds balls into channel after they are intaked**/
    int channel_id; /**< Talon id of the channel motor **/
    double current_limit; /**< The current limit (amps) of the talons **/
    double serializer_cmd; /**< percent output to serializing talon when it is turned on **/
    double channel_cmd; /**< percent output to channel talon when it is turned on **/
    int s0_port; /**< we use sensors to track the balls in the channel. this is one of them **/
    int s1_port; /**< we use sensors to track the balls in the channel. this is one of them **/
    int s2_port; /**< antiquated, we don't use this sensor **/
    int s3_port; /**< antiquated, we don't use this sensor **/
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
    units::radian_t angle_above_horizontal; /**< limelight isn't exaclty level; this is the offset angle **/
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

void SetFramePeriodsForPidTalon(TalonSRX& talon, FeedbackType feedback_type = FeedbackType::None);
void SetFramePeriodsForPidTalonFX(TalonFX& talon, FeedbackType feedback_type = FeedbackType::None);

void SetFramePeriodsForOpenLoopTalon(TalonSRX& talon);

void SetFramePeriodsForSlaveTalon(TalonSRX& talon);
void SetFramePeriodsForSlaveTalonFX(TalonFX& talon);

}  // namespace conf
}  // namespace c2020
}  // namespace team114
