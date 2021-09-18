#include "Shoot.h"
#include <string>


Shoot::Shoot(){
    shoot_slave->Follow(*shoot_master);
}

//Limelight aiming and stuff
void Shoot::Periodic(){
    if (state == State::Idle) {
        limelight->setLEDMode("OFF");
        std::cout <<"idle\n";
    } 
    if (state == State::Aiming) {
        limelight->setLEDMode("ON");
        Aim();
    }
    if (state == State::Shooting) {
        limelight->setLEDMode("ON");
        //Check aim
        //shoot
    }
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}

//Aim would align turret and hood angle not shooter speed
void Shoot::Aim(){
    //PID constants ~13 lbs
    READING_SDB_NUMERIC(double, Turret_P) TKi;
    READING_SDB_NUMERIC(double, Turret_I) TKp;

	x_off = limelight->getXOff();
  //  TKp = 0.01;
  //  TKi = 0;
	double power = TKp*x_off + TKi;
    if (power < -0.99) power = -0.99;
    if (power > 0.99) power = 0.99;

    frc::SmartDashboard::PutNumber("power turret rot", power);

    //Things with turret a bit sus so I'm commenting this out
    //turret->Set(ControlMode::PercentOutput, power);
    if (x_off < 2) state = State::Shooting;
}

void Shoot::Auto(){}

void Shoot::Manual(const frc::XboxController & xbox){
    double shoot_value = xbox.GetY(frc::GenericHID::kLeftHand);
    double turret_rot = xbox.GetX(frc::GenericHID::kRightHand);
    double hood_value = xbox.GetY(frc::GenericHID::kRightHand);

    //deadband
    if(shoot_value <= 0.05){
        shoot_value = 0;
    }
    if(abs(turret_rot) <= 0.05){
        turret_rot = 0;
    }
    if(hood_value <= 0.05){
        hood_value = 0;
    }

    //0-1
    servo_left.Set(hood_value);
    servo_right.Set(hood_value);

    //0-1
    shoot_master->Set(ControlMode::PercentOutput, shoot_value*2);
    shoot_slave->Set(ControlMode::PercentOutput, -shoot_value*2);
  //  turret->Set(ControlMode::PercentOutput, -turret_rot*0.1);
}

void Shoot::Calibration(){
    frc::SmartDashboard::PutNumber("flywheel", flywheel_speed);
    frc::SmartDashboard::PutNumber("hood", hood_angle);


    servo_left.Set(hood_angle);
    servo_right.Set(hood_angle);
}