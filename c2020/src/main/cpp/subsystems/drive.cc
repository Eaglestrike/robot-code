#include "drive.h"

#include <thread>

#include <frc/SPI.h>
#include <frc/Timer.h>
#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>

#include <units/units.h>

#include "util/number_util.h"

namespace team114 {
namespace c2020 {

Drive::Drive() : Drive{conf::GetConfig().drive} {}

Drive::Drive(const conf::DriveConfig& cfg)
    : left_master_{cfg.left_master_id},
      right_master_{cfg.right_master_id},
      left_slave_{cfg.left_slave_id},
      right_slave_{cfg.right_slave_id},
      config_{cfg},
      robot_state_{RobotState::GetInstance()},
      kinematics_{cfg.track_width},
      odometry_{{}},
      ramsete_{} {
    conf::DriveFalconCommonConfig(left_master_);
    conf::DriveFalconCommonConfig(right_master_);
    conf::DriveFalconCommonConfig(left_slave_);
    conf::DriveFalconCommonConfig(right_slave_);
    left_master_.SetInverted(true);
    right_master_.SetInverted(false);
    left_slave_.Follow(left_master_);
    right_slave_.Follow(right_master_);
    left_slave_.SetInverted(InvertType::FollowMaster);
    right_slave_.SetInverted(InvertType::FollowMaster);

    conf::SetDriveMasterFramePeriods(left_master_);
    conf::SetDriveMasterFramePeriods(right_master_);
    conf::SetDriveSlaveFramePeriods(left_slave_);
    conf::SetDriveSlaveFramePeriods(right_slave_);
    CheckFalconFramePeriods();
}

// https://phoenix-documentation.readthedocs.io/en/latest/ch18_CommonAPI.html#can-bus-utilization-error-metrics
void Drive::CheckFalconFramePeriods() {
    if (left_master_.HasResetOccurred()) {
        conf::SetDriveMasterFramePeriods(left_master_);
        falcon_reset_count_++;
    }
    if (right_master_.HasResetOccurred()) {
        conf::SetDriveMasterFramePeriods(right_master_);
        falcon_reset_count_++;
    }
    if (left_slave_.HasResetOccurred()) {
        conf::SetDriveSlaveFramePeriods(left_slave_);
        falcon_reset_count_++;
    }
    if (right_slave_.HasResetOccurred()) {
        conf::SetDriveSlaveFramePeriods(right_slave_);
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
    WaitForNavxCalibration(0.5);
    navx_.ZeroYaw();
    left_master_.SetSelectedSensorPosition(0);
    right_master_.SetSelectedSensorPosition(0);
    odometry_.ResetPosition({}, GetYaw());
}

void Drive::WaitForNavxCalibration(double timeout_sec) {
    frc::Timer time;
    time.Start();
    while (navx_.IsCalibrating() && time.Get() < timeout_sec) {
        // LOG calib retry
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (navx_.IsCalibrating()) {
        // LOG failed to calibrate
    } else {
        // LOG success
    }
}

void Drive::OutputTelemetry() {}

void Drive::SetWantDriveTraj(frc::Trajectory traj) {}

void Drive::UpdateRobotState() {}

void Drive::UpdatePathController() {}

bool Drive::FinishedTraj() {
    // TODO(josh)
    return false;
}

void Drive::SetWantRawOpenLoop(
    const frc::DifferentialDriveWheelSpeeds& openloop) {
    state_ = DriveState::OPEN_LOOP;
    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = openloop.left.to<double>();
    pout_.right_demand = openloop.right.to<double>();
}

// stolen from 254
void Drive::SetWantCheesyDrive(double throttle, double wheel, bool quick_turn) {
    throttle = Deadband(throttle, 0.05);
    wheel = Deadband(wheel, 0.035);

    constexpr double kWheelGain = 1.97;
    constexpr double kWheelNonlinearity = 0.05;
    const double denominator = std::sin(M_PI / 2.0 * kWheelNonlinearity);
    // Apply a sin function that's scaled to make it feel better.
    if (!quick_turn) {
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = wheel / (denominator * denominator) * std::abs(throttle);
    }
    wheel *= kWheelGain;
    frc::DifferentialDriveWheelSpeeds signal = kinematics_.ToWheelSpeeds(
        frc::ChassisSpeeds{units::meters_per_second_t{throttle}, 0.0_mps,
                           units::radians_per_second_t{wheel}});
    signal.Normalize(1.0_mps);
    SetWantRawOpenLoop(signal);
}

void Drive::WriteOuts() {
    SDB_NUMERIC(double, LeftDriveTalonDemand){pout_.left_demand};
    SDB_NUMERIC(double, RightDriveTalonDemand){pout_.right_demand};
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
