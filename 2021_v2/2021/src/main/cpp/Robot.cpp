#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);

  //camera = frc::CameraServer::GetInstance()->StartAutomaticCapture();
}


void Robot::RobotPeriodic() {
  compressor.Start();
}


void Robot::AutonomousInit() {
  _shooter.Zero();
  Auto_timer.Reset();
  Auto_timer.Start();
}


void Robot::AutonomousPeriodic() {

  if(Auto_timer.Get() < 2){
    _drivetrain.Auto();
  } 
  else if(Auto_timer.Get() > 3 && Auto_timer.Get() < 5){
    _shooter.Aim();
    _shooter.setState(Shoot::State::Shooting);
  }
  else if(Auto_timer.Get() > 5 && Auto_timer.Get() < 7){
    _channel.setState(Channel::State::Shooting);
  }
  else if(Auto_timer.Get() > 10){
    _shooter.setState(Shoot::State::Idle);
    _channel.setState(Channel::State::Idle);
  }

  _shooter.Periodic();
  _channel.Periodic();
}


void Robot::TeleopInit() {
  Auto_timer.Reset();
  Auto_timer.Stop();
}


void Robot::TeleopPeriodic() {
  //std::cout << "here" << std::endl;
  //Drive & Manual turret movement
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));
  if (_climb.climbing)
    _climb.Manual_Climb(xbox.GetY(frc::GenericHID::kLeftHand));

  //Aim Button A
  if(xbox.GetRawButton(1)){
      _shooter.setState(Shoot::State::Aiming);
      _channel.setState(Channel::State::Idle);
      _intake.setState(Intake::State::Idle);
  }

  //Shoot Button B
  else if(xbox.GetRawButton(2)) {
    _shooter.setState(Shoot::State::Shooting);
    _channel.setState(Channel::State::Idle);
    Auto_timer.Start();
      if(Auto_timer.Get() > 1.5){
      _channel.setState(Channel::State::Shooting);
      }
  }

  //intake Button X
  else if(xbox.GetRawButton(3)){
    _intake.setState(Intake::State::Deploy);
    _channel.setState(Channel::State::Intake);
    _shooter.setState(Shoot::State::Idle);
  } 

  //Manual Zero
  else if(xbox.GetBackButton()){
    _shooter.Manual_Zero();
  }

  //Climb
  else if(l_joy.GetTrigger() && r_joy.GetTrigger()){
    _climb.climbing = true;
    _climb.Extend();
  }  


  else {
    _intake.setState(Intake::State::Idle);
    _shooter.setState(Shoot::State::Idle);
    _channel.setState(Channel::State::Idle);
  }  
  if (!xbox.GetRawButton(2)) {
    Auto_timer.Reset();
    Auto_timer.Stop();
  }

  _shooter.Periodic();
  _channel.Periodic();
  _intake.Periodic();
}


void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}
void Robot::TestInit() {}


//Shooter Calibration
void Robot::TestPeriodic() {
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));
  
  //Shoot Button B
  if(xbox.GetRawButton(2)) {
    _shooter.setState(Shoot::State::Calibrate);
    _channel.setState(Channel::State::Idle);
    Auto_timer.Start();
      if(Auto_timer.Get() > 1.5){
      _channel.setState(Channel::State::Shooting);
      }
  }

  else if(xbox.GetRawButton(1)){
    _shooter.Turret_Calibrate();
  }

  else {
    _intake.setState(Intake::State::Idle);
    _shooter.setState(Shoot::State::Idle);
    _channel.setState(Channel::State::Idle);
  }  
  if (!xbox.GetRawButton(2)) {
    Auto_timer.Reset();
    Auto_timer.Stop();
  }

  _shooter.Periodic();
  _channel.Periodic();
  _intake.Periodic();
}


#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif