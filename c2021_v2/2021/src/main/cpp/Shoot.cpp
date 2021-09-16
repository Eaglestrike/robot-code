#include "Shoot.h"
#include "sdb_types.h"
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
    TKp = 0.017; 
    TKi = 0.015;
	double x_off = limelight->getXOff();

	double power = TKp*x_off + TKi;
    if (power < -0.99) power = -0.99;
    if (power > 0.99) power = 0.99;
  //  std::cout <<"x offset: " << x_off << std::endl; 
   // std::cout << "steering adjust: " << steering_adjust << std::endl; 
    turret->Set(ControlMode::PercentOutput, power);

    if (x_off < 2) state = State::Shooting;
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

void Shoot::Calibration() {
    READING_SDB_NUMERIC(double, flywheel_speed) fwspd; //can add pid when calibrating
    READING_SDB_NUMERIC(double, hood_angle) hdang; //can add pid when calibrating
  
    READING_SDB_NUMERIC(double, turret_pos) turrpos;
    READING_SDB_NUMERIC(double, turretP) tp; TKp = tp;
    READING_SDB_NUMERIC(double, turretI) ti; TKi = ti;

}