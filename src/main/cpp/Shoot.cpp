#include "Shoot.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/Timer.h>
#include <string>
#include <map>

//frc::Timer* timer;
std::map <double, double> output_to_time;
double max_out = 0.9; //can adjust 

Shoot::Shoot(){
    shoot_slave->Follow(*shoot_master);
    turret->ConfigMotionCruiseVelocity(500);
    turret->ConfigMotionAcceleration(500);
    output_to_time_init();
}

void Shoot::output_to_time_init() {
    //set stuff
}

double output_to_time_f(double flywheel_out) {
    double bestT;
	std::map<double, double>::iterator it;
	it = output_to_time.begin();
	for (it = output_to_time.begin(); it != output_to_time.end(); it++) //iterate through settings
	{
		double prevOut = it->first;
		double nextOut = (std::next(it, 1))->first;
		double prevT = it->second;
		double nextT = (std::next(it, 1))->second;
        if (flywheel_out < prevOut) return prevT; //min threshold
		if (prevOut <= flywheel_out && flywheel_out <= nextOut) { //we want something between these settings
			//it's the previous setting plus the difference to the next setting times the percent (0-1) to the next setting the input distance is
            return bestT = prevT + (nextT - prevT)*((flywheel_out-prevOut)/(nextOut-prevOut));
        }
	}
	return output_to_time[max_out];
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
            //timer->Stop();
            //timer->Reset();
            shoot_master->Set(ControlMode::PercentOutput, 0.0);
            shoot_slave->Set(ControlMode::PercentOutput, 0.0);
            break;
        case State::Aiming:
            //limelight->setLEDMode("ON");
            Aim();
            break;
        case State::Shooting:
            //if (timer->Get() == 0) timer->Start(); //check to make sure a paused, reset timer returns 0
            //limelight->setLEDMode("ON");
            //Its_gonna_shoot();

        default:
            break;
    }
    
    /*if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        turret->Set(ControlMode::PercentOutput, 0.0);
    }*/
    
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}


void Shoot::Its_gonna_shoot(){
    //AutoShoot::Settings s;
    //s = AutoShootCalc(limelight.getNetworkTable());
    //set hood
    //set flywheel speed
    //once flywheel is up to speed, set kicker speed
    //set channel to run

    // servo_left.Set(hood_out); //set hood
    // servo_right.Set(hood_out);
    // shoot_master->Set(ControlMode::PercentOutput, flywheel_out);
    // if (abs(output_to_time_f(flywheel_out) - timer->Get()) > 0.1) return; //adjust tolerance
}

int prev_xoff = 0;
void Shoot::Aim() {
    READING_SDB_NUMERIC(double, Turret_P) TKp;
    READING_SDB_NUMERIC(double, Turret_I) TKi;
    READING_SDB_NUMERIC(double, Turret_D) TKd;
    // TKp = 0.015;
    // TKi = -0.013;
    // TKd = 0.009;
	x_off = limelight->getXOff();
    double delta_xoff = (x_off - prev_xoff);
	double power = TKp*x_off + TKi + TKd*delta_xoff;
    prev_xoff = x_off;
    if (power < -0.5) power = -0.5;
    if (power > 0.5) power = 0.5;
    if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        power = 0;
    }
    turret->Set(ControlMode::PercentOutput, power);
}

void Shoot::Shooter_Calibrate(){
    READING_SDB_NUMERIC(double, Hood_angle_out) hood_out;
    READING_SDB_NUMERIC(double, Flywheel_percent_out) flywheel_out;
    //frc::SmartDashboard::PutNumber("limelight_distance", );

    shoot_master->Set(ControlMode::PercentOutput, flywheel_out);
    shoot_slave->Set(ControlMode::PercentOutput, -flywheel_out);
    servo_left.Set(hood_out);
    servo_right.Set(hood_out);
}

void Shoot::Auto(){
    
}

// void Shoot::Manual(const frc::XboxController & xbox){
//     double shoot_value = xbox.GetY(frc::GenericHID::kLeftHand);
//     double hood_value = xbox.GetY(frc::GenericHID::kRightHand);
//     if(shoot_value <= 0.09){
//         shoot_value = 0;
//     }
//     if(hood_value <= 0.09){
//         hood_value = 0;
//     }
//     shoot_master->Set(ControlMode::PercentOutput, shoot_value*2);
//     shoot_slave->Set(ControlMode::PercentOutput, -shoot_value*2);
//     Manual_Turret(xbox);
// }

void Shoot::Zero(){
    while(turret_limit_switch->Get()){
        turret->Set(ControlMode::PercentOutput, 0.10);
    }
    turret->SetSelectedSensorPosition(0); //zero for motion magic
    turret->Set(ControlMode::PercentOutput, 0);
}

void Shoot::Manual_Turret(double turret_rot){
    if(abs(turret_rot) <= 0.05){
        turret_rot = 0;
    }
    if(turret_rot > 0 && turret->GetSelectedSensorPosition() > 0){
        turret_rot = 0;
    }
    if(turret_rot < 0 && turret->GetSelectedSensorPosition() < -19870){
        turret_rot = 0;
    }
    turret->Set(ControlMode::PercentOutput, turret_rot*0.12);
}