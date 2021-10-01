#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
}

void Robot::RobotPeriodic() {
  compressor.Start();
}

void Robot::AutonomousInit() {
  Auto_timer.Reset();
  Auto_timer.Start();
}
void Robot::AutonomousPeriodic() {
  if(Auto_timer.Get() < 2){
    _drivetrain.Auto();
  } else {
    _drivetrain.Stop();
    _shooter.Aim();
    if(Auto_timer.Get() < 7){
      _shooter.Its_gonna_shoot();
    }
  }
}
void Robot::TeleopInit() {
  Auto_timer.Stop();
  _shooter.Zero();
}

void Robot::TeleopPeriodic() {
  
  //Drive & Manual turret movement
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));

  //Aim
  if(xbox.GetRawButton(1)){
      _shooter.setState(Shoot::State::Aiming);
  } else {
      _shooter.setState(Shoot::State::Idle);
  }

  //Shoot
  if(xbox.GetRawButton(3)){
    _shooter.setState(Shoot::State::Shooting);
    _channel.setState(Channel::State::Shooting);
  } else {
    _channel.setState(Channel::State::Idle);
    _shooter.setState(Shoot::State::Idle);
  }

  //Intake 
  if(xbox.GetRawButton(4)){
    _intake.setState(Intake::State::Deploy);
    _channel.setState(Channel::State::Intake);
  } else {
    _channel.setState(Channel::State::Idle);
    _intake.setState(Intake::State::Idle);
  }

  _shooter.Periodic();
  _channel.Periodic();
  _intake.Periodic();

  //Climb
  if(l_joy.GetTrigger() && r_joy.GetTrigger()){
    std::cout << "Climb" << std::endl;
    //Do climbing stuff
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
