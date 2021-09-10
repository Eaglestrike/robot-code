#include "Robot.h"
#include <iostream>
#include <string>

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);

  //Motor Initialization
  left_slave->Follow(*left_master);
  right_slave->Follow(*right_master);

  left_master->SetInverted(TalonFXInvertType::CounterClockwise);
  right_master->SetInverted(TalonFXInvertType::Clockwise);
  left_slave->SetInverted(TalonFXInvertType::FollowMaster);
  right_slave->SetInverted(TalonFXInvertType::FollowMaster);
}

void Robot::RobotPeriodic() {}
void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {}
void Robot::TeleopInit() {}

void Robot::TeleopPeriodic() {
  double forward = l_joy.GetRawAxis(1);
  double turn = r_joy.GetRawAxis(0);

  m_drive.ArcadeDrive(forward, turn, false);

  m_drive.SetRightSideInverted(false);

  // if(xbox.GetRawButton(1)==1){
  //   std::cout << "xbox controller getting input" << std::endl;
  // }
  std::cout << "right joystick: " << r_joy.GetRawAxis(0) << r_joy.GetRawAxis(1) << std::endl;
  std::cout << "left joystick: "<< l_joy.GetRawAxis(0) << l_joy.GetRawAxis(1) << std::endl;
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
