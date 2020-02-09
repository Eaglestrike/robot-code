#pragma once

#include <frc/geometry/Pose2d.h>

#include "config.h"
#include "subsystem.h"
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

    std::pair<units::second_t, frc::Pose2d> GetLatestFieldToRobot();
    frc::Pose2d GetFieldToRobot(units::second_t);
    void ObserveFieldToRobot(units::second_t timestamp,
                             const frc::Pose2d& pose);
    void ResetFieldToRobot();

   private:
    InterpolatingMap<units::second_t, frc::Pose2d,
                     ArithmeticInverseInterp<units::second_t>, Pose2dInterp>
        field_to_robot_;
};

}  // namespace c2020
}  // namespace team114
