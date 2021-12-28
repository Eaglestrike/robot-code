
#include "Robot.h"
#include <iostream>
#include <frc/smartdashboard/SmartDashboard.h>

void Robot::RobotInit() {
  try{
    navx = new AHRS(frc::SPI::Port::kMXP);
  } catch(const std::exception& e){
    std::cout << e.what() <<std::endl;
  }

  
}

void Robot::RobotPeriodic() {}
void Robot::AutonomousInit() {}

void Robot::AutonomousPeriodic() {
  m_swerve.UpdateOdometry();
}

void Robot::TeleopInit() {}

void Robot::TeleopPeriodic() {
  m_swerve.UpdateOdometry();
  m_swerve.GetRotation(navx->GetRotation2d());
  Drive(true);

  rotation = m_swerve.GetPose().Rotation();
  position = m_swerve.GetPose().Translation();
  //std::cout << "Position  X: " << position.X() << "  Y: " << position.Y() << std::endl;
  //std::cout << "Rotation: " << rotation.Degrees() << std::endl;
  
  if(xbox.GetRawButton(1)){
    std::cout << "Set PID for swerve modules" << std::endl;
    m_swerve.SetPID(); 
  }
}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}
void Robot::TestPeriodic() {}



void Robot::Drive(bool fieldRelative){
  double xjoy = abs(l_joy.GetRawAxis(0)) < 0.05 ? 0.0 : l_joy.GetRawAxis(0);
  double yjoy = abs(l_joy.GetRawAxis(1)) < 0.05 ? 0.0 : l_joy.GetRawAxis(1);
  double rotjoy = abs(r_joy.GetRawAxis(0)) < 0.05 ? 0.0 : r_joy.GetRawAxis(0);

   const auto xSpeed = xjoy * Drivetrain::MaxSpeed;
   const auto ySpeed = yjoy * Drivetrain::MaxSpeed;
   const auto rot = -rotjoy * Drivetrain::MaxAngularSpeed;

  m_swerve.Drive(xSpeed, ySpeed, rot, fieldRelative);
}



#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif
