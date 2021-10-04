#include "Robot.h"

void Robot::RobotInit() {
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);

  //camera = frc::CameraServer::GetInstance()->StartAutomaticCapture();
}

void Robot::RobotPeriodic() {
  compressor.Start();
  /*if(!compressor.GetPressureSwitchValue()){
    compressor.Start();
  } else {
    compressor.Stop();
  }*/
}

void Robot::AutonomousInit() {
  _shooter.Zero();
  Auto_timer.Reset();
  Auto_timer.Start();
}

void Robot::AutonomousPeriodic() {
  if(Auto_timer.Get() < 2){
    //std::cout << "drive" << std::endl;
    _drivetrain.Auto();
  } 
  else if(Auto_timer.Get() > 3 && Auto_timer.Get() < 5){
    //std::cout << "move flywheel" << std::endl;
    _shooter.Aim();
    _shooter.setState(Shoot::State::Shooting);
    _shooter.Periodic();
  }
  else if(Auto_timer.Get() > 5 && Auto_timer.Get() < 7){
    //std::cout << "move channel" << std::endl;
    _channel.setState(Channel::State::Shooting);
    _channel.Periodic();
  }
  else if(Auto_timer.Get() > 10){
    _shooter.setState(Shoot::State::Idle);
    _shooter.Periodic();
    _channel.setState(Channel::State::Idle);
    _channel.Periodic();
  }
}

void Robot::TeleopInit() {
  Auto_timer.Reset();
  Auto_timer.Stop();
}

void Robot::TeleopPeriodic() {
  //Drive & Manual turret movement
  _shooter.Manual_Turret(xbox.GetX(frc::GenericHID::kRightHand));
  _drivetrain.Periodic(l_joy.GetRawAxis(1), -1*r_joy.GetRawAxis(0));
  
  //Aim Button A
  if(xbox.GetRawButton(1)){
      _shooter.setState(Shoot::State::Aiming);
      _channel.setState(Channel::State::Idle);
      _intake.setState(Intake::State::Idle);
  }

  //Shoot Button B
  else if(xbox.GetRawButton(2)) {
    _shooter.setState(Shoot::State::Shooting);
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

  // //Climb
  // else if(l_joy.GetTrigger() && r_joy.GetTrigger()){
  //   std::cout << "Climb" << std::endl;
  //   //Do climbing stuff
  // } 
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
