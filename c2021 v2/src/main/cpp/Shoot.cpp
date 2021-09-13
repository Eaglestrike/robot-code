#include "Shoot.h"

Shoot::Shoot(){
    // shoot_master->Set(TalonFXInvertType::Clockwise);
    // shoot_slave->Set(TalonFXInvertType::Clockwise);
    shoot_slave->Follow(*shoot_master);
}

void Shoot::Periodic(const frc::XboxController & xbox){
    shoot_master->Set(ControlMode::PercentOutput, xbox.GetRawAxis(frc::GenericHID::kRightHand)*2);
    shoot_slave->Set(ControlMode::PercentOutput, -xbox.GetRawAxis(frc::GenericHID::kRightHand)*2);
}

void Shoot::Aim(){

}

void Shoot::Auto(){

}