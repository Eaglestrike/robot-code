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

struct HoodConfig {
    int talon_id;
    double ticks_per_degree;
    double max_degrees;
    double current_limit;
    double zeroing_kp;
    double zeroing_vel;
    double kA;
    double kV;
    double kP;
    double kD;
};

struct RobotConfig {
    std::string mac_address;
    DriveConfig drive;
    HoodConfig hood;
};

RobotConfig& GetConfig();

void DriveFalconCommonConfig(TalonFX& falcon);

void SetDriveMasterFramePeriods(TalonFX& falcon);

void SetDriveSlaveFramePeriods(TalonFX& falcon);

}  // namespace conf
}  // namespace c2020
}  // namespace team114
