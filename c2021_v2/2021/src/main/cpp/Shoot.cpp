#include "Shoot.h"
#include <string>

Shoot::Shoot(){
    // shoot_master->Set(TalonFXInvertType::Clockwise);
    // shoot_slave->Set(TalonFXInvertType::Clockwise);
    shoot_slave->Follow(*shoot_master);
}

//Limelight aiming and stuff
//We can't even test due to no serializer so... idk have percent output on dashboard when we test
void Shoot::Periodic(){
    if (state == State::Idle) {
        limelight->setLEDMode("OFF");
    } 
    if (state == State::Aiming) {
        limelight->setLEDMode("ON");
        Aim();
    }
    if (state == Shooting) {
        limelight->setLEDMode("ON");
        //set hood
        // set flywheel
    }
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}

void Shoot::Aim(){
    double Kp = 0.017; 
    double Ki = 0.015;
	double x_off = limelight->getXOff();

	double power = Kp*x_off + Ki;
    if (power < -0.99) power = -0.99;
    if (power > 0.99) power = 0.99;
  //  std::cout <<"x offset: " << x_off << std::endl; 
   // std::cout << "steering adjust: " << steering_adjust << std::endl; 
    turret->Set(ControlMode::PercentOutput, power);
}

void Shoot::Auto(){}

void Shoot::Manual(const frc::XboxController & xbox){
    double shoot_value = xbox.GetY(frc::GenericHID::kLeftHand);
    double turret_rot = xbox.GetY(frc::GenericHID::kRightHand);

    double hood_value = xbox.GetX(frc::GenericHID::kRightHand);

    //deadband
    if(shoot_value <= 0.05){
        shoot_value = 0;
    }
    if(abs(turret_rot) <= 0.05){
        turret_rot = 0;
    }
    if(abs(hood_value) <= 0.05){
        hood_value = 0;
    }

    shoot_master->Set(ControlMode::PercentOutput, shoot_value*2);
    shoot_slave->Set(ControlMode::PercentOutput, -shoot_value*2);
    turret->Set(ControlMode::PercentOutput, -turret_rot*0.15);


};
