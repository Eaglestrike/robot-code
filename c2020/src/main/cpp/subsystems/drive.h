#pragma once

#include <optional>

#include "shims/minimal_phoenix.h"

#include <frc/Timer.h>
#include <frc/controller/ProfiledPIDController.h>
#include <frc/controller/RamseteController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>

#include <units/units.h>

#include "config.h"
#include "robot_state.h"
#include "shims/navx_ahrs.h"
#include "subsystem.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class Drive : public Subsystem {
    SUBSYSTEM_PRELUDE(Drive)
   public:
    Drive(const conf::DriveConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    void SetWantDriveTraj(frc::Trajectory&& traj);
    bool FinishedTraj();

    // Assumes range of -1 to 1 mps max speed
    void SetWantRawOpenLoop(const frc::DifferentialDriveWheelSpeeds& openloop);
    void SetWantCheesyDrive(double throttle, double wheel, bool quick_turn);

    void SetWantOrientForShot(Limelight& limelight);
    bool OrientedForShot();

   private:
    enum class DriveState {
        OPEN_LOOP,
        FOLLOW_PATH,
        SHOOT_ORIENT,
    };
    struct PeriodicOut {
        ControlMode control_mode;
        double left_demand;
        double right_demand;
    };
    void CheckFalconFramePeriods();

    void UpdateRobotState();
    void UpdatePathController();
    void UpdateOrientController();

    frc::Rotation2d GetYaw();
    units::meter_t GetEncoder(TalonFX& master_talon);
    void WaitForNavxCalibration(double timeout_sec);

    SDB_NUMERIC(unsigned int, DriveFalconResetCount) falcon_reset_count_{0};

    TalonFX left_master_, right_master_;
    TalonFX left_slave_, right_slave_;
    AHRS navx_{frc::SPI::Port::kMXP};

    PeriodicOut pout_{};
    void WriteOuts();

    const conf::DriveConfig cfg_;
    DriveState state_{DriveState::OPEN_LOOP};
    RobotState& robot_state_;

    frc::DifferentialDriveKinematics kinematics_;
    frc::DifferentialDriveOdometry odometry_;
    frc::RamseteController ramsete_;
    std::optional<frc::Trajectory> curr_traj_;
    frc2::Timer traj_timer{};

    frc::ProfiledPIDController<units::radian> vision_rot_;
    bool has_vision_target_;
};

}  // namespace c2020
}  // namespace team114
