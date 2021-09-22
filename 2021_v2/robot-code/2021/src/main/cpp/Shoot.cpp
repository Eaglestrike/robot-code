#include "Shoot.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <string>

Shoot::Shoot(){
    shoot_slave->Follow(*shoot_master);
    turret->ConfigMotionCruiseVelocity(500);
    turret->ConfigMotionAcceleration(500);
}

void Shoot::Periodic(){
    if (turret->GetStatorCurrent() > 45) {
        turret->Set(ControlMode::PercentOutput, 0.0);
        std::cout << "Current exceeded!" << std::endl;
        return; //don't set again
    }
    switch(state){
        case State::Idle:
            //limelight->setLEDMode("OFF");
            //add manual turret here
            break;
        case State::Aiming:
            //limelight->setLEDMode("ON");
            Aim();
            break;
        case State::Shooting:
            limelight->setLEDMode("ON");
        default:
            break;
    }
    
    if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        turret->Set(ControlMode::PercentOutput, 0.0);
    }
    
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}

//Aim would align turret and hood angle not shooter speed
void Shoot::Aim() {
    READING_SDB_NUMERIC(double, Turret_P) TKp;
    READING_SDB_NUMERIC(double, Turret_I) TKi;

	x_off = limelight->getXOff();
	double power = 0.016*x_off + 0.008;
    if (power < -0.5) power = -0.5;
    if (power > 0.5) power = 0.5;

    if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        power = 0;
    }
    turret->Set(ControlMode::PercentOutput, power);
}

void Shoot::Auto(){}

void Shoot::Manual(const frc::XboxController & xbox){
    double shoot_value = xbox.GetY(frc::GenericHID::kLeftHand);
    double hood_value = xbox.GetY(frc::GenericHID::kRightHand);

    if(shoot_value <= 0.05){
        shoot_value = 0;
    }
    if(hood_value <= 0.05){
        hood_value = 0;
    }

    shoot_master->Set(ControlMode::PercentOutput, shoot_value*2);
    shoot_slave->Set(ControlMode::PercentOutput, -shoot_value*2);
    Manual_Turret(xbox);
}

void Shoot::Calibration(){
    /*frc::SmartDashboard::PutNumber("flywheel", flywheel_speed);
    frc::SmartDashboard::PutNumber("hood", hood_angle);
    servo_left.Set(hood_angle);
    servo_right.Set(hood_angle);*/
}

void Shoot::Zero(){
    while(turret_limit_switch->Get()){
        turret->Set(ControlMode::PercentOutput, 0.10);
    }
    turret->SetSelectedSensorPosition(0); //zero for motion magic
    turret->Set(ControlMode::PercentOutput, 0);
}

void Shoot::Manual_Turret(const frc::XboxController & xbox){
    double turret_rot = xbox.GetX(frc::GenericHID::kRightHand);
    if(abs(turret_rot) <= 0.05){
        turret_rot = 0;
    }
    if(turret_rot > 0 && turret->GetSelectedSensorPosition() > 0){
        turret_rot = 0;
    }
    if(turret_rot < 0 && turret->GetSelectedSensorPosition() < -19870){
        turret_rot = 0;
    }
    turret->Set(ControlMode::PercentOutput, turret_rot*0.1);
}