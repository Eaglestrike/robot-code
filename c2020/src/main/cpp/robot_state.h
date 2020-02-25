#pragma once

#include <utility>

#include <frc/geometry/Pose2d.h>

#include "config.h"
#include "subsystem.h"
#include "subsystems/limelight.h"
#include "util/constructor_macros.h"
#include "util/interp_map.h"

namespace team114 {
namespace c2020 {

// Saying frame1_to_frame2 represents the transform applied to the frame1 origin
// that will bring it to the frame2 origin.
class RobotState {
    DISALLOW_COPY_ASSIGN(RobotState)
    CREATE_SINGLETON(RobotState)
   public:
    RobotState();
    RobotState(conf::RobotConfig cfg);

    std::pair<units::second_t, frc::Pose2d> GetLatestFieldToRobot();
    frc::Pose2d GetFieldToRobot(units::second_t);
    void ObserveFieldToRobot(units::second_t timestamp,
                             const frc::Pose2d& pose);
    void ResetFieldToRobot();

    void ObserveVision(units::second_t timestamp,
                       std::optional<Limelight::TargetInfo> target);
    std::optional<std::pair<units::second_t, units::meter_t>>
    GetLatestDistanceToOuterPort();
    std::optional<std::pair<units::second_t, units::radian_t>>
    GetLatestAngleToOuterPort();

   private:
    InterpolatingMap<units::second_t, frc::Pose2d,
                     ArithmeticInverseInterp<units::second_t>, Pose2dInterp>
        field_to_robot_;

    const conf::LimelightConfig ll_cfg_;
    std::optional<std::pair<units::second_t, Limelight::TargetInfo>>
        debounced_vision_;
    const unsigned int kDroppableFrames = 3;
    unsigned int vision_null_ct_;
};

}  // namespace c2020
}  // namespace team114
