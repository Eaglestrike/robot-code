
#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
  frc::SmartDashboard::PutBoolean("Manual Mode", Manual_mode);
}

void Robot::RobotPeriodic() {}
void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {
  _control.Auto();
}
void Robot::TeleopInit() {}

//Gotta add compressor stuff
void Robot::TeleopPeriodic() {
  if(xbox.GetBackButtonPressed()){
    Manual_mode = !Manual_mode;
  }
  
  if(Manual_mode){
    _control.Manual(xbox, l_joy, r_joy);
  }
  else {
    _control.Periodic(xbox, l_joy, r_joy);
  }

  //_control.Testing(xbox, l_joy, r_joy);

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