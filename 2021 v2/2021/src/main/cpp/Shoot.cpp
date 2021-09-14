#include "Shoot.h"

Shoot::Shoot(){
    // shoot_master->Set(TalonFXInvertType::Clockwise);
    // shoot_slave->Set(TalonFXInvertType::Clockwise);
    shoot_slave->Follow(*shoot_master);
}

//Limelight aiming and stuff
//We can't even test due to no serializer so... idk have percent output on dashboard when we test
void Shoot::Periodic(const frc::XboxController & xbox){

}

void Shoot::Aim(){}
void Shoot::Auto(){}

void Shoot::Manual(const frc::XboxController & xbox){
    double shoot_value = xbox.GetRawAxis(2);
    double turret_rot = xbox.GetRawAxis(4);
    //deadband
    if(shoot_value <= 0.05){
        shoot_value = 0;
    }
    if(abs(turret_rot) <= 0.05){
        turret_rot = 0;
    }
    shoot_master->Set(ControlMode::PercentOutput, shoot_value*2);
    shoot_slave->Set(ControlMode::PercentOutput, -shoot_value*2);
    turret->Set(ControlMode::PercentOutput, turret_rot*0.5);
};