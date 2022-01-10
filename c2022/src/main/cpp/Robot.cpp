#include "Robot.h"
#include <iostream>
#include <frc/smartdashboard/SmartDashboard.h>

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);

  try{
    navx = new AHRS(frc::SPI::Port::kMXP);
  } catch(const std::exception& e){
    std::cout << e.what() <<std::endl;
  }
}

void Robot::RobotPeriodic() {}
void Robot::AutonomousInit() {
  m_autoSelected = m_chooser.GetSelected();
  // m_autoSelected = SmartDashboard::GetString("Auto Selector",
  //     kAutoNameDefault);
  std::cout << "Auto selected: " << m_autoSelected << std::endl;

  if (m_autoSelected == kAutoNameCustom) {
    // Custom Auto goes here
  } else {
    // Default Auto goes here
  }
}

void Robot::AutonomousPeriodic() {
  if (m_autoSelected == kAutoNameCustom) {
    // Custom Auto goes here
  } else {
    // Default Auto goes here
  }
}

void Robot::TeleopInit() {}


void Robot::TeleopPeriodic() {

  //Deadband 
  double x1, y1, x2;
  x1 = l_joy.GetRawAxis(0);
  y1 = l_joy.GetRawAxis(1) * 0.6;
  x2 = r_joy.GetRawAxis(0);
  x1 = abs(x1) < 0.05 ? 0.0: x1;
  y1 = abs(y1) < 0.05 ? 0.0: y1;
  x2 = abs(x2) < 0.05 ? 0.0: x2;

  frc::SmartDashboard::PutNumber("x1",x1);
  frc::SmartDashboard::PutNumber("y1",y1);
  frc::SmartDashboard::PutNumber("x2",x2);

  m_swerve.Drive(x1, y1, x2, navx->GetYaw(), true);

  //For Setting PID Values (A)
  if(xbox.GetRawButton(1)){
    std::cout << "Set PID for swerve modules" << std::endl;
    m_swerve.SetPID(); 
  }

  //Resetting the gryo for field Oriented Drive (B)
  if(xbox.GetRawButton(2)){
    std::cout << "reset gryo" << std::endl;
    navx->Reset();
  }

  //Resetting the encoders
  if(xbox.GetRawButton(3)){
    std::cout << "reset Encoders" << std::endl;
    m_swerve.ResetEncoders();
  }
}


void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}

//TODO Andrew
//Robot Oriented Drive For testing
void Robot::TestPeriodic() {}



#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif