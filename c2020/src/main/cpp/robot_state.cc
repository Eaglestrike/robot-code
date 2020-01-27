#include "robot_state.h"

#include <frc/geometry/Pose2d.h>

namespace team114 {
namespace c2020 {

RobotState::RobotState() : field_to_robot_{800} {}

std::pair<double, frc::Pose2d> RobotState::GetLatestFieldToRobot() {
    return field_to_robot_.Latest();
}

void RobotState::ObserveFieldToRobot(double timestamp,
                                     const frc::Pose2d& pose) {
    field_to_robot_[timestamp] = pose;
}

void RobotState::ResetFieldToRobot() { field_to_robot_.Inner().clear(); }

}  // namespace c2020
}  // namespace team114
