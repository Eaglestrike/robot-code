#include "robot_state.h"

#include <frc/geometry/Pose2d.h>

namespace team114 {
namespace c2020 {
    

/**
 * default construtor of robot state, calls the other constructor(?)
 */
RobotState::RobotState() : RobotState{conf::GetConfig()} {}

/**
 * configures the robot state w/ field position, limelight configuration, and
 * vision systems
 */
RobotState::RobotState(conf::RobotConfig cfg)
    : field_to_robot_{800},
      ll_cfg_{cfg.limelight},
      debounced_vision_{},
      vision_null_ct_{0} {}

/**
 * gets the latest known field position of the robot
 * @returns latest field to robot/position value
 */
std::pair<units::second_t, frc::Pose2d> RobotState::GetLatestFieldToRobot() {
    return field_to_robot_.Latest();
}

/**
 * gets the field positon of the robot at a given timestamp
 * @returns field to robot/position and a given timestamp
 */
frc::Pose2d RobotState::GetFieldToRobot(units::second_t timestamp) {
    return field_to_robot_.InterpAt(timestamp).second;
}

/**
 * Reads the stance/position of the robot at a given time and sets it to an
 * array(timeline like?)
 */
void RobotState::ObserveFieldToRobot(units::second_t timestamp,
                                     const frc::Pose2d& pose) {
    field_to_robot_[timestamp] = pose;
}

/**
 * resets the robot's field positioning
 */
void RobotState::ResetFieldToRobot() { field_to_robot_.Inner().clear(); }

/**
 * Checks the robot's vision, resets if it can't find the target too many times
 */
void RobotState::ObserveVision(units::second_t timestamp,
                               std::optional<Limelight::TargetInfo> target) {
    if (!target.has_value()) {
        if (vision_null_ct_ > kDroppableFrames) {
            debounced_vision_.reset();
        }
        vision_null_ct_++;
        return;
    }
    vision_null_ct_ = 0;
    debounced_vision_ = std::make_pair(timestamp, target.value());
}

/**
 * calculates the cotangent of a given value
 * @returns cotangent of given value
 */
inline units::scalar_t FastCotangent(units::radian_t r) {
    auto const pi2 = units::radian_t{M_PI_2};
    return units::math::tan(pi2 - r);
}

std::optional<std::pair<units::second_t, units::meter_t>>
/**
 * Gets the latest recorded distance (to the outer port?) from the robot vision if possible
 * @returns the distance
 */
RobotState::GetLatestDistanceToOuterPort() {
    if (!debounced_vision_.has_value()) {
        return {};
    }
    auto target = *debounced_vision_;
    auto dist =
        ll_cfg_.diff_height *
        FastCotangent(target.second.vertical + ll_cfg_.angle_above_horizontal);
    return std::make_pair(target.first, dist);
}

std::optional<std::pair<units::second_t, units::radian_t>>
/**
 * Gets/calculates the angle at which the robot's vision is viewing an object(the outer port?)
 * @returns the angle
 */
RobotState::GetLatestAngleToOuterPort() {
    if (!debounced_vision_.has_value()) {
        return {};
    }
    auto target = *debounced_vision_;
    return std::make_pair(target.first, target.second.horizontal);
}

}  // namespace c2020
}  // namespace team114
