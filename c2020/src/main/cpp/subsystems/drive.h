#pragma once

#include <ctre/Phoenix.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>

#include "config.h"
#include "subsystem.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class Drive : public Subsystem {
    SUBSYSTEM_PRELUDE(Drive)
   public:
    Drive(const DriveConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

   private:
    void CheckFalconFramePeriods();
    SDB_NUMERIC(unsigned int, FalconResetCount) falcon_reset_count_;

    TalonFX left_master_, right_master_;
    TalonFX left_slave_, right_slave_;
    DriveConfig config_;
    frc::DifferentialDriveKinematics kinematics_;
};

}  // namespace c2020
}  // namespace team114
