#include "drive.h"

#include <frc/Timer.h>
#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>

#include <units/units.h>

namespace team114 {
namespace c2020 {

Drive::Drive() : Drive{GetConfig().drive} {}

Drive::Drive(const DriveConfig& cfg)
    : falcon_reset_count_{0},
      left_master_{cfg.left_master_id},
      right_master_{cfg.right_master_id},
      left_slave_{cfg.left_slave_id},
      right_slave_{cfg.right_slave_id},
      navx_{SPI::Port::kMXP},
      pout_{},
      config_{cfg},
      state_{DriveState::OPEN_LOOP},
      robot_state_{RobotState::GetInstance()},
      kinematics_{cfg.track_width},
      odometry_{{}},
      ramsete_{} {
    DriveFalconCommonConfig(left_master_);
    DriveFalconCommonConfig(right_master_);
    DriveFalconCommonConfig(left_slave_);
    DriveFalconCommonConfig(right_slave_);
    // TODO(josh) configure setinverted and sensor polarity

    SetDriveMasterFramePeriods(left_master_);
    SetDriveMasterFramePeriods(right_master_);
    SetDriveSlaveFramePeriods(left_slave_);
    SetDriveSlaveFramePeriods(right_slave_);
    // TODO(josh) see if HasResetOccurred is always true on first call
    CheckFalconFramePeriods();
}

// https://phoenix-documentation.readthedocs.io/en/latest/ch18_CommonAPI.html#can-bus-utilization-error-metrics
void Drive::CheckFalconFramePeriods() {
    if (left_master_.HasResetOccurred()) {
        SetDriveMasterFramePeriods(left_master_);
        falcon_reset_count_++;
    }
    if (right_master_.HasResetOccurred()) {
        SetDriveMasterFramePeriods(right_master_);
        falcon_reset_count_++;
    }
    if (left_slave_.HasResetOccurred()) {
        SetDriveSlaveFramePeriods(left_slave_);
        falcon_reset_count_++;
    }
    if (right_slave_.HasResetOccurred()) {
        SetDriveSlaveFramePeriods(right_slave_);
        falcon_reset_count_++;
    }
}

void Drive::Periodic() {
    CheckFalconFramePeriods();
    odometry_.Update(GetYaw(), GetEncoder(left_master_),
                     GetEncoder(right_master_));
    robot_state_.ObserveFieldToRobot(frc::Timer::GetFPGATimestamp(),
                                     odometry_.GetPose());
    switch (state_) {
        case (DriveState::OPEN_LOOP):
            // TODO(josh)
            break;
        case (DriveState::FOLLOW_PATH):
            UpdatePathController();
            break;
        default:
            // TODO(josh) log here
            break;
    }
    WriteOuts();
}

void Drive::Stop() {}

void Drive::ZeroSensors() {
    robot_state_.ResetFieldToRobot();
    navx_.ZeroYaw();
    left_master_.SetSelectedSensorPosition(0);
    right_master_.SetSelectedSensorPosition(0);
    odometry_.ResetPosition({}, GetYaw());
}

void Drive::OutputTelemetry() {}

void Drive::SetWantDriveTraj(frc::Trajectory traj) {}

void Drive::UpdateRobotState() {}

void Drive::UpdatePathController() {}

bool Drive::FinishedTraj() {
    // TODO(josh)
    return false;
}

void Drive::SetWantRawOpenLoop(const Signal& sig) {
    state_ = DriveState::OPEN_LOOP;
    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = sig.left;
    pout_.right_demand = sig.right;
}

void Drive::WriteOuts() {
    left_master_.Set(pout_.control_mode, pout_.left_demand);
    right_master_.Set(pout_.control_mode, pout_.right_demand);
}

constexpr auto RAD_PER_DEGREE = units::constants::pi * 1_rad / 180.0;
frc::Rotation2d Drive::GetYaw() {
    return frc::Rotation2d(navx_.GetYaw() * RAD_PER_DEGREE);
}

units::meter_t Drive::GetEncoder(TalonFX& master_talon) {
    return (static_cast<double>(master_talon.GetSelectedSensorPosition())) *
           config_.meters_per_falcon_tick;
}

}  // namespace c2020
}  // namespace team114
