#include "robot.h"

#include <units/units.h>

namespace team114 {
namespace c2020 {

Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      drive_{Drive::GetInstance()},
      robot_state_{RobotState::GetInstance()},
      ljoy_{0},
      rjoy_{1} {}

void Robot::RobotInit() {}
void Robot::RobotPeriodic() { drive_.Periodic(); }

void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {}

void Robot::TeleopInit() {}
void Robot::TeleopPeriodic() {
    SDB_NUMERIC(double, JoystickThrottle) throttle = ljoy_.GetRawAxis(1);
    SDB_NUMERIC(double, JoystickWheel) wheel = -rjoy_.GetRawAxis(0);
    SDB_BOOL(JoystickQuickturn) quick_turn = rjoy_.GetRawButton(1);
    drive_.SetWantCheesyDrive(throttle, wheel, quick_turn);
}

void Robot::TestInit() {}
void Robot::TestPeriodic() {}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}

}  // namespace c2020
}  // namespace team114

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<team114::c2020::Robot>(); }
#endif
