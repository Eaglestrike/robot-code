#pragma once

#include <optional>

#include <AHRS.h>
#include <ctre/Phoenix.h>
#include <frc/Timer.h>
#include <frc/controller/RamseteController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>

#include "config.h"
#include "robot_state.h"
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

    void SetWantDriveTraj(frc::Trajectory traj);
    bool FinishedTraj();

    struct Signal {
        double left;
        double right;
    };

    void SetWantRawOpenLoop(const Signal& openloop);

   private:
    enum class DriveState {
        OPEN_LOOP,
        FOLLOW_PATH,
    };
    struct PeriodicOut {
        ControlMode control_mode;
        double left_demand;
        double right_demand;
    };
    void CheckFalconFramePeriods();

    void UpdateRobotState();
    void UpdatePathController();

    frc::Rotation2d GetYaw();
    units::meter_t GetEncoder(TalonFX& master_talon);

    SDB_NUMERIC(unsigned int, FalconResetCount) falcon_reset_count_;

    TalonFX left_master_, right_master_;
    TalonFX left_slave_, right_slave_;
    AHRS navx_;

    PeriodicOut pout_;
    void WriteOuts();

    DriveConfig config_;
    DriveState state_;
    RobotState& robot_state_;

    frc::DifferentialDriveKinematics kinematics_;
    frc::DifferentialDriveOdometry odometry_;
    frc::RamseteController ramsete_;
    std::optional<frc::Trajectory> curr_traj_;
    double traj_start_time_ = 0.0;
};

}  // namespace c2020
}  // namespace team114
