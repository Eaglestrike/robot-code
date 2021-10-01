#include "Shoot.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/Timer.h>
#include <string>
#include <map>

// frc::Timer* timer;
// std::map <double, double> output_to_time;
// double max_out = 0.9; //can adjust 

Shoot::Shoot(){
    shoot_slave->Follow(*shoot_master);
    turret->ConfigMotionCruiseVelocity(500);
    turret->ConfigMotionAcceleration(500);
    turret->SetExpiration(5);
    shoot_slave->SetExpiration(5);
    shoot_master->SetExpiration(5);

    dataMap[-3.8] = {0.36, 0.79};
	dataMap[-1.2] = {0.37, 0.77};
	dataMap[1.0] = {0.39, 0.72};
	dataMap[3.0] = {0.39, 0.7};
	dataMap[4.57] = {0.38, 0.68};
	dataMap[5.47] = {0.37, 0.65};
	dataMap[9.5] = {0.35, 0.62};
    dataMap[14.06] = {0.32, 0.59};
    dataMap[18.2] = {0.06, 0.58};
    dataMap[20.1] = {0.05, 0.57};
    dataMap[22.4] = {0.0, 0.53};
    //output_to_time_init();
}

//void Shoot::output_to_time_init() {
    //set stuff
//}

//double output_to_time_f(double flywheel_out) {
    // double bestT;
	// std::map<double, double>::iterator it;
	// it = output_to_time.begin();
	// for (it = output_to_time.begin(); it != output_to_time.end(); it++) //iterate through settings
	// {
	// 	double prevOut = it->first;
	// 	double nextOut = (std::next(it, 1))->first;
	// 	double prevT = it->second;
	// 	double nextT = (std::next(it, 1))->second;
    //     if (flywheel_out < prevOut) return prevT; //min threshold
	// 	if (prevOut <= flywheel_out && flywheel_out <= nextOut) { //we want something between these settings
	// 		//it's the previous setting plus the difference to the next setting times the percent (0-1) to the next setting the input distance is
    //         return bestT = prevT + (nextT - prevT)*((flywheel_out-prevOut)/(nextOut-prevOut));
    //     }
	// }
	// return output_to_time[max_out];
//}

void Shoot::Periodic(){
    switch(state){
        case State::Idle:
            limelight->setLEDMode("OFF");
            shoot_master->Set(ControlMode::PercentOutput, 0.0);
            shoot_slave->Set(ControlMode::PercentOutput, 0.0);
            break;
        case State::Aiming:
            limelight->setLEDMode("ON");
            Aim();
            break;
        case State::Shooting:
            limelight->setLEDMode("ON");
            Its_gonna_shoot();
        default:
            break;
    }    
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}


void Shoot::Its_gonna_shoot(){
    double point = limelight->getYOff();
	double angle, speed;

	auto data = dataMap.find(point);
	if (data != dataMap.end()) { 
		// point is in dataMap. get exact angle and speed from hash map
		angle = data->second.first;
		speed = data->second.second;
	} else { 
	  // point is not in dataMap. Find 2 closest points for interploation
	  double point1, point2;
		if (interpolate(dataPoints, point, point1, point2)) {
			// Found 2 points for interpolation
			auto data1 = dataMap[point1];
			double angle1, speed1;
			angle1 = data1.first;
			speed1 = data1.second;
			
			auto data2 = dataMap[point2];
			double angle2, speed2;
			angle2 = data2.first;
			speed2 = data2.second;

			// interpolate
			angle = (angle1 + angle2) / 2;
			speed = (speed1 + speed2) / 2;

		} else {
			return;
		}
	}
    shoot_master->Set(ControlMode::PercentOutput, speed);
    shoot_slave->Set(ControlMode::PercentOutput, -speed);
    servo_left.Set(angle);
    servo_right.Set(angle);
//     AutoShoot::Settings s;
//   //  s = AutoShootCalc(limelight.getNetworkTable());


//      servo_left.Set(hood_out); //set hood
//      servo_right.Set(hood_out);
    // shoot_master->Set(ControlMode::PercentOutput, flywheel_out);
    // if (abs(output_to_time_f(flywheel_out) - timer->Get()) > 0.1) return; //adjust tolerance
    
}


int prev_xoff = 0;
void Shoot::Aim() {
    //READING_SDB_NUMERIC(double, Turret_P) TKp;
    //READING_SDB_NUMERIC(double, Turret_I) TKi;
    //READING_SDB_NUMERIC(double, Turret_D) TKd;
    TKp = 0.015;
    TKi = -0.013;
    TKd = 0.009;
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

    shoot_master->Set(ControlMode::PercentOutput, flywheel_out);
    shoot_slave->Set(ControlMode::PercentOutput, -flywheel_out);
    servo_left.Set(hood_out);
    servo_right.Set(hood_out);
}


double Shoot::GetLimelightY(){
    return limelight->getYOff();
}


void Shoot::Auto(){}


void Shoot::Zero(){
    while(turret_limit_switch->Get()){
        turret->Set(ControlMode::PercentOutput, 0.10);
    }
    turret->SetSelectedSensorPosition(0);
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


bool Shoot::interpolate(std::vector<double>& array, double p, double& p1, double& p2){
    int left = 0, right = array.size()-1;

	if (p < array[left] || p > array[right]) {
		return false;
	}

	int mid = (left+right)/2;

	while (left < right) {
		if (array[mid] <= p && p <= array[mid+1]) {
			p1 = array[mid];
			p2 = array[mid+1];
			return true;
		} else if (array[mid] <= p) {
			left = mid;
		} else {
			right = mid;
		}
		mid = (left+right)/2;
	}
	return false;
}