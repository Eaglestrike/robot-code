#include "Shoot.h"
#include "Limelight.h"

Shoot::Shoot(){
    // shoot_master->Set(TalonFXInvertType::Clockwise);
    // shoot_slave->Set(TalonFXInvertType::Clockwise);
    shoot_slave->Follow(*shoot_master);
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}

//Limelight aiming and stuff
//We can't even test due to no serializer so... idk have percent output on dashboard when we test
void Shoot::Periodic(const frc::XboxController & xbox){
    if (state == State::Idle) {
        //reset? idk
    } 
    if (state == State::Aiming) {
        Aim();
    }
    if (state == Shooting) {
        //set hood
        // set flywheel
    }
}

/* For testing:
    - should Kp, x_off, Ki each be positive or negative?
    - tune pid
*/
void Shoot::Aim() {
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