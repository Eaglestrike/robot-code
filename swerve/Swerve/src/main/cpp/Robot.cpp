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

  //TODO Andrew - add deadband
  //Deadband 
  // double x1, y1, x2;
  // x1 = (abs(l_joy.GetRawAxis(0)) < 0.002) ? 0.0: l_joy.GetRawAxis(0);
  // y1 = (abs(l_joy.GetRawAxis(1)) < 0.002) ? 0.0: l_joy.GetRawAxis(1);
  // x2 = (abs(r_joy.GetRawAxis(0)) < 0.002) ? 0.0: r_joy.GetRawAxis(0);
  // std::cout << l_joy.GetRawAxis(0) << std::endl;
  
  m_swerve.fieldOrientDrive(l_joy.GetRawAxis(0), l_joy.GetRawAxis(1), 
    r_joy.GetRawAxis(0), navx->GetYaw());

  if(xbox.GetRawButton(1)){
    std::cout << "Set PID for swerve modules" << std::endl;
    m_swerve.SetPID(); 
  }

  if(xbox.GetRawButton(2)){
    std::cout << "reset gryo" << std::endl;
    navx->Reset();
  }
}


void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}

//TODO Andrew
//Robot Oriented Drive For testing
void Robot::TestPeriodic() {
  m_swerve.robotOrientDrive(l_joy.GetRawAxis(0), l_joy.GetRawAxis(1), 
    r_joy.GetRawAxis(0));

  if(xbox.GetRawButton(2)){
    std::cout << "reset gryo" << std::endl;
    navx->Reset();
  }  

  frc::SmartDashboard::PutNumber("navx", navx->GetYaw());
}



#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif
