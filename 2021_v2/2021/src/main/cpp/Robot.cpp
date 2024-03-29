#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
  
  //camera = frc::CameraServer::GetInstance()->StartAutomaticCapture();

  try{
    navx = new AHRS(frc::SPI::Port::kMXP);
  } catch(const std::exception& e){
    std::cout << e.what() <<std::endl;
  }
}


void Robot::RobotPeriodic() {
  compressor.Start();
}


void Robot::AutonomousInit() {
  Auto_timer.Reset();
  Auto_timer.Start();
  navx->ZeroYaw();
  _shooter.Auto();
}


void Robot::AutonomousPeriodic() {
  
  if(Auto_timer.Get() > 1 && Auto_timer.Get() < 3){
    _drivetrain.Auto();
  } 
  else if(Auto_timer.Get() > 4 && Auto_timer.Get() < 8){
    _shooter.Aim(navx->GetYaw());
    _shooter.setState(Shoot::State::Shooting);
  }
  else if(Auto_timer.Get() > 8 && Auto_timer.Get() < 13){
    _channel.setState(Channel::State::Shooting);
  }
  else if(Auto_timer.Get() > 13){
    _shooter.setState(Shoot::State::Idle);
    _channel.setState(Channel::State::Idle);
  }

  _shooter.Periodic(navx->GetYaw());
  _channel.Periodic();
}


void Robot::TeleopInit() {
  Auto_timer.Reset();
  Auto_timer.Stop();
}


void Robot::TeleopPeriodic() {
  //Drive & Manual turret movement
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));

  //Enabling Climb operator left joystick
  if(_climb.climbing){
    _climb.Manual_Climb(xbox.GetY(frc::GenericHID::kLeftHand));
  }
  
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
      if(Auto_timer.Get() > 1.0 && _shooter.target_found){
        _channel.setState(Channel::State::Shooting);
        _intake.setState(Intake::State::Shoot);
      }
  }

  //intake Button X
  else if(xbox.GetRawButton(3)){
    _intake.setState(Intake::State::Deploy);
    _channel.setState(Channel::State::Intake);
    _shooter.setState(Shoot::State::Idle);
  } 

  else if(l_joy.GetTriggerPressed() &&r_joy.GetTriggerPressed()){
    _intake.setState(Intake::State::Climb);
    _climb.climbing = true;
    _climb.Extend();
  }

  //Manual Zero
  else if(xbox.GetBackButton()){
    _shooter.Manual_Zero();
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

  _shooter.Periodic(navx->GetYaw());
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
      if(Auto_timer.Get() > 2.0){
      _channel.setState(Channel::State::Shooting);
      }
  }

  //Aim Button A
  else if(xbox.GetRawButton(1)){
    _shooter.Turret_Calibrate();
  }

  else if(l_joy.GetTrigger()){
    _drivetrain.navx_testing(navx->GetYaw());
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

  _shooter.Periodic(navx->GetYaw());
  _channel.Periodic();
  _intake.Periodic();

  frc::SmartDashboard::PutNumber("yaw", navx->GetYaw());
}


#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif