#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
}

void Robot::RobotPeriodic() {}
void Robot::AutonomousInit() {}

void Robot::AutonomousPeriodic() {
  Drivetrain.Auto();
}
void Robot::TeleopInit() {}

void Robot::TeleopPeriodic() {
  Drivetrain.Periodic(l_joy, r_joy);
  Shooter.Periodic(xbox);
}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}
void Robot::TestPeriodic() {}


#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif
