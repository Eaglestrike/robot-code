#include "drive.h"

#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>

namespace team114 {
namespace c2020 {

Drive::Drive() : Drive{GetConfig().drive} {}

Drive::Drive(const DriveConfig& cfg)
    : falcon_reset_count_{0},
      left_master_{cfg.left_master_id},
      right_master_{cfg.right_master_id},
      left_slave_{cfg.left_slave_id},
      right_slave_{cfg.right_slave_id},
      config_{cfg},
      kinematics_{cfg.track_width} {
    DriveFalconCommonConfig(left_master_);
    DriveFalconCommonConfig(right_master_);
    DriveFalconCommonConfig(left_slave_);
    DriveFalconCommonConfig(right_slave_);

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

void Drive::Periodic() { CheckFalconFramePeriods(); }

void Drive::Stop() {}

void Drive::ZeroSensors() {}

void Drive::OutputTelemetry() {}

}  // namespace c2020
}  // namespace team114
