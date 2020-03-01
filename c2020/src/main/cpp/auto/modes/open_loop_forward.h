#pragma once

#include "../action.h"
#include "../actions/control_flow.h"
#include "../actions/drive_actions.h"
#include "../actions/shoot.h"

#include <units/units.h>

#include <paths.h>

namespace team114 {
namespace c2020 {
namespace auton {

inline std::unique_ptr<Action> MakeOpenLoopForward() {
    return std::make_unique<DriveOpenLoopAction>(
        frc::DifferentialDriveWheelSpeeds{0.25_mps, 0.25_mps}, 3.0_s);
}

inline std::unique_ptr<Action> MakeShootAndOLReverse() {
    return std::make_unique<SeriesAction>(MakeActionList(
        std::make_unique<ShootAction>(8.0_s),
        std::make_unique<DriveOpenLoopAction>(
            frc::DifferentialDriveWheelSpeeds{0.25_mps, 0.25_mps}, 3.0_s)));
}

}  // namespace auton
}  // namespace c2020
}  // namespace team114
