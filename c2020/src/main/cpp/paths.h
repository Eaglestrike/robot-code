#pragma once

#include <memory>

#include <frc/trajectory/TrajectoryGenerator.h>

namespace team114 {
namespace c2020 {
namespace paths {

frc::TrajectoryConfig MakeDefaultConfig();

frc::Trajectory TestPath();

}  // namespace paths
}  // namespace c2020
}  // namespace team114
