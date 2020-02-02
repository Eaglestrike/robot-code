#include "paths.h"

#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/trajectory/constraint/CentripetalAccelerationConstraint.h>

#include "config.h"

#define CACHING_PATH_BODY(body)                             \
    static std::optional<frc::Trajectory> __cached;         \
    if (!__cached.has_value()) {                            \
        __cached.emplace(([]() -> frc::Trajectory body)()); \
    }                                                       \
    return __cached.value();

namespace team114 {
namespace c2020 {
namespace paths {

using frc::Pose2d;
using frc::Translation2d;

frc::TrajectoryConfig MakeDefaultConfig() {
    conf::DriveConfig& drive_cfg = conf::GetConfig().drive;
    frc::DifferentialDriveKinematics kinematics{drive_cfg.track_width};
    frc::TrajectoryConfig traj_cfg{drive_cfg.traj_max_vel,
                                   drive_cfg.traj_max_accel};
    // traj_cfg.SetKinematics(kinematics);
    traj_cfg.AddConstraint(
        frc::DifferentialDriveKinematicsConstraint(kinematics, 3.0_mps));
    traj_cfg.AddConstraint(frc::CentripetalAccelerationConstraint(
        drive_cfg.traj_max_centrip_accel));
    return traj_cfg;
}

frc::Trajectory TestPath() {
    CACHING_PATH_BODY({
        auto cfg = MakeDefaultConfig();
        return frc::TrajectoryGenerator::GenerateTrajectory(
            Pose2d{0.0_m, 0.0_m, {0.0_rad}},
            {
                {0.5_m, 1.0_m},
                {1.0_m, 1.0_m},
                {1.5_m, 1.0_m},
            },
            Pose2d{2.0_m, 0.0_m, {0.0_rad}}, cfg);
    })
}

}  // namespace paths
}  // namespace c2020
}  // namespace team114
