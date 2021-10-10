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
    turret->SetExpiration(30);
    shoot_slave->SetExpiration(30);
    shoot_master->SetExpiration(30);

    dataMap[-3.8] = {0.47, 0.95};
	dataMap[-1.2] = {0.44, 0.95};
	dataMap[1.0] = {0.45, 0.92};
	dataMap[3.0] = {0.44, 0.92};
	dataMap[4.57] = {0.43, 0.92};
	dataMap[5.47] = {0.42, 0.92};
	dataMap[9.5] = {0.41, 0.92};
    dataMap[14.06] = {0.40, 0.90};
    dataMap[18.2] = {0.20, 0.90};
    dataMap[20.1] = {0.12, 0.90};
    dataMap[22.4] = {0.0, 0.90};

    // dataMap[-3.8] = {0.36, 0.79};
	// dataMap[-1.2] = {0.37, 0.77};
	// dataMap[1.0] = {0.39, 0.72};
	// dataMap[3.0] = {0.39, 0.7};
	// dataMap[4.57] = {0.38, 0.68};
	// dataMap[5.47] = {0.37, 0.65};
	// dataMap[9.5] = {0.35, 0.62};
    // dataMap[14.06] = {0.32, 0.59};
    // dataMap[18.2] = {0.06, 0.58};
    // dataMap[20.1] = {0.05, 0.57};
    // dataMap[22.4] = {0.0, 0.53};

   
}



void Shoot::Periodic(){
    switch(state){
        case State::Idle:
            limelight->setLEDMode("ON");
            shoot_master->Set(ControlMode::PercentOutput, 0.0);
            shoot_slave->Set(ControlMode::PercentOutput, 0.0);
            //turret->SetNeutralMode(NeutralMode::Brake);
            break;
        case State::Aiming:
            limelight->setLEDMode("ON");
            Aim();
            break;
        case State::Shooting:
            turret->SetNeutralMode(NeutralMode::Brake);
            limelight->setLEDMode("ON");
            Its_gonna_shoot();
        default:
            break;
    }    
}

void Shoot::setState(Shoot::State newState) {
    state = newState;
}


bool Shoot::Its_gonna_shoot(){
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
			return false;
		}
	}
    //std::cout << "flywheel output: " << speed << std::endl;
    shoot_master->Set(ControlMode::PercentOutput, speed);
    shoot_slave->Set(ControlMode::PercentOutput, -speed);
    servo_left.Set(angle);
    servo_right.Set(angle);
    return true;
//     AutoShoot::Settings s;
//   //  s = AutoShootCalc(limelight.getNetworkTable());


//      servo_left.Set(hood_out); //set hood
//      servo_right.Set(hood_out);
    // shoot_master->Set(ControlMode::PercentOutput, flywheel_out);
    // if (abs(output_to_time_f(flywheel_out) - timer->Get()) > 0.1) return; //adjust tolerance
    
}

void Shoot::ShortShot() {
    //eventually get rid of arguments and use constants
    //aim?
    shoot_master->Set(ControlMode::Velocity, 0.0);
    if (shoot_master->GetSelectedSensorVelocity() - 0.0 < 100) { //adjust
        servo_left.Set(0.0);
        servo_right.Set(0.0);
    }
    
}


int prev_xoff = 0;
void Shoot::Aim() {
    READING_SDB_NUMERIC(double, Turret_P) TKp;
    READING_SDB_NUMERIC(double, Turret_I) TKi;
    READING_SDB_NUMERIC(double, Turret_D) TKd;
   
   // TKp = 0.015; //just this by itself is ok
 
	x_off = limelight->getXOff();
    double power;
    // if(x_off > 500){
    //     if (turret->GetSelectedSensorPosition() < -19870.0/2.0) power = 0.25;
    //     if (turret->GetSelectedSensorPosition() > -19870.0/2.0) power = -0.25;
    // }
    if (x_off > 500) { //give up
        power = 0;
       // turret->SetNeutralMode(NeutralMode::Brake);
    }
    double delta_xoff = (x_off - prev_xoff);

	power = TKp*x_off + TKi + TKd*delta_xoff;
    prev_xoff = x_off;
    
    if (power < -0.5) power = -0.5;
    if (power > 0.5) power = 0.5;
    if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        power = 0;
    }
     std::cout << power << std::endl;
    turret->Set(ControlMode::PercentOutput, power);

     std::cout << "x_off: " << x_off << std::endl;
     std::cout << "power: " << power << std::endl;
}


void Shoot::Shooter_Calibrate(){
    READING_SDB_NUMERIC(double, Hood_angle_out) hood_out;
    READING_SDB_NUMERIC(double, Flywheel_speed) flywheel_out;

    shoot_master->Set(ControlMode::Velocity, flywheel_out);
    shoot_slave->Set(ControlMode::Velocity, -flywheel_out);

    servo_left.Set(hood_out);
    servo_right.Set(hood_out);
}


double Shoot::GetLimelightY(){
    return limelight->getYOff();
}

double Shoot::GetLimelightX(){
    return limelight->getXOff();
}

void Shoot::Auto(){
    
}


void Shoot::Zero(){
    // while(turret_limit_switch->Get()){
    //     turret->Set(ControlMode::PercentOutput, 0.12);
    // }
    turret->SetSelectedSensorPosition(0);
    turret->Set(ControlMode::PercentOutput, 0);
    //Please work
    while(turret->GetSelectedSensorPosition() > -9500){
        turret->Set(ControlMode::PercentOutput, -0.1);
    }
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

void Shoot::Unjam(){
    // servo_left.Set(1.0);
    // servo_right.Set(1.0);
    shoot_master->Set(ControlMode::PercentOutput, 0.5);
    //shoot_slave->Set(ControlMode::PercentOutput, -0.5);
}