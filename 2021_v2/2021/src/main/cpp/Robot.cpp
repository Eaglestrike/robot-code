#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
}

void Robot::RobotPeriodic() {

}

void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {}
void Robot::TeleopInit() {}

void Robot::TeleopPeriodic() {
  if(xbox.GetBackButtonPressed()){
    Manual_mode = !Manual_mode;
    if(Manual_mode){
      std::cout << "Manual\n\n\n\n"; 
    }
    else {
      std::cout << "Periodic\n\n\n\n"; 
    }
  }
  
  if(Manual_mode){
    _control.Manual(xbox, l_joy, r_joy);
  }
  else {
  _control.Periodic(xbox, l_joy, r_joy);
  }
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
