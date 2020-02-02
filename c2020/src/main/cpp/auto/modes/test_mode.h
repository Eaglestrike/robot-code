#pragma once

#include "../action.h"
#include "../actions/control_flow.h"
#include "../actions/drive_actions.h"

#include <paths.h>

namespace team114 {
namespace c2020 {
namespace auton {

inline std::unique_ptr<Action> MakeTestMode() {
    return std::make_unique<DrivePathAction>(paths::TestPath());
}

}  // namespace auton
}  // namespace c2020
}  // namespace team114
