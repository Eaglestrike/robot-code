#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
}

void Robot::RobotPeriodic() {
  compressor.Start();
}

void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {
  _drivetrain.Auto();
}
void Robot::TeleopInit() {
  _shooter.Zero();
}

void Robot::TeleopPeriodic() {
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));

   _drivetrain.SetWantCheesyDrive(-l_joy.GetRawAxis(1), r_joy.GetRawAxis(0), r_joy.GetRawButton(1));

  //Button 1 is A
  if(xbox.GetRawButton(1)){
      _shooter.setState(Shoot::State::Aiming);
      _shooter.Periodic(); //shouldn't this be outside an if statement??
  } else {
      _shooter.setState(Shoot::State::Idle);
  }

  //Button 2 is B?
  if(xbox.GetRawButton(2)){
      _channel.Run();
      _shooter.Shooter_Calibrate();
  } else {
      _channel.Stop();
      _shooter.setState(Shoot::State::Idle);
      _shooter.Periodic();
  }
}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}
void Robot::TestPeriodic() {
}


#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif