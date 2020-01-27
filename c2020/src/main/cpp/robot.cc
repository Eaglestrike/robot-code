#include "robot.h"

#include <units/units.h>

namespace team114 {
namespace c2020 {

Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      drive_{Drive::GetInstance()},
      robot_state_{RobotState::GetInstance()} {}

void Robot::RobotInit() {}
void Robot::RobotPeriodic() { drive_.Periodic(); }

void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {}

void Robot::TeleopInit() {}
void Robot::TeleopPeriodic() {}

void Robot::TestInit() {}
void Robot::TestPeriodic() {
    drive_.SetWantRawOpenLoop(Drive::Signal{0.1, 0.1});
}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}

}  // namespace c2020
}  // namespace team114

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<team114::c2020::Robot>(); }
#endif
