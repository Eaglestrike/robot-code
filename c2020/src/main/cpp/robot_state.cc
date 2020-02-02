#include "robot_state.h"

#include <frc/geometry/Pose2d.h>

namespace team114 {
namespace c2020 {

RobotState::RobotState() : field_to_robot_{800} {}

std::pair<units::second_t, frc::Pose2d> RobotState::GetLatestFieldToRobot() {
    return field_to_robot_.Latest();
}

frc::Pose2d RobotState::GetFieldToRobot(units::second_t timestamp) {
    return field_to_robot_.InterpAt(timestamp).second;
}

void RobotState::ObserveFieldToRobot(units::second_t timestamp,
                                     const frc::Pose2d& pose) {
    field_to_robot_[timestamp] = pose;
}

void RobotState::ResetFieldToRobot() { field_to_robot_.Inner().clear(); }

}  // namespace c2020
}  // namespace team114
