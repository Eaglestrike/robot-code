#include "Shoot.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/Timer.h>
#include <string>
#include <map>


//Shooter constructor
Shoot::Shoot(){
    //Configure motors
    //shoot_slave->Follow(*shoot_master);
    turret->ConfigMotionCruiseVelocity(500);
    turret->ConfigMotionAcceleration(500);
    turret->SetExpiration(30);
    shoot_slave->SetExpiration(30);
    shoot_master->SetExpiration(30);
    shoot_master->SetSafetyEnabled(false);
    shoot_slave->SetSafetyEnabled(false);
    shoot_master->Config_kP(0, 0.25, 30);
    shoot_master->Config_kI(0, 0.0, 30);
    shoot_master->Config_kD(0, 0.0, 30);
    shoot_slave->Config_kP(0, 0.25, 30);
    shoot_slave->Config_kI(0, 0.0, 30);
    shoot_slave->Config_kD(0, 0.0, 30);

    //Hash map for hood and flywheel values
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
}


void Shoot::Periodic(double robot_yaw){
    turret->SetNeutralMode(NeutralMode::Brake);
    switch(state){
        case State::Idle:
            limelight->setLEDMode("ON");
            shoot_master->Set(ControlMode::PercentOutput, 0.0);
            shoot_slave->Set(ControlMode::PercentOutput, 0.0);
            break;
        case State::Aiming:
            limelight->setLEDMode("ON");
            Aim(robot_yaw);
            shoot_master->Set(ControlMode::PercentOutput, 0.70);
            shoot_slave->Set(ControlMode::PercentOutput, -0.70);
            break;
        case State::Shooting:
            limelight->setLEDMode("ON");
            Its_gonna_shoot();
            break;
        case State::Calibrate:
            Shooter_Calibrate();
            break;
        default:
            break;
    }    
}


//Set shooter states
void Shoot::setState(Shoot::State newState) {
    state = newState;
}


//Shoot - Find values from the limelight and set shooter values
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
    std::cout << "angle" << angle << std::endl;
    shoot_master->Set(ControlMode::PercentOutput, speed);
    shoot_slave->Set(ControlMode::PercentOutput, -speed);
    servo_left.Set(angle);
    servo_right.Set(angle);
    return true;
}


//Aim the turret to the goal
double prev_xoff = 0;
double acc_error = 0;
void Shoot::Aim(double robot_yaw) {

    if(!limelight->target_valid()){
        FindTarget(robot_yaw);
    }
    else {
        //Pid for turret speed output
        x_off = limelight->getXOff();
        double power;
        if (x_off > 500) {
            power = 0;
        }

        double TKp = 0.017;
        double TKi = 0.0;
        double TKd = 0.015;

        double delta_xoff = (x_off - prev_xoff);
        acc_error += x_off;

        double heading_error = x_off;
        power = 0.0;
        power = TKp*heading_error + TKi*acc_error + TKd*delta_xoff;
        prev_xoff = x_off;
        
        if (power < -0.25) power = -0.25;
        if (power > 0.25) power = 0.25;
        if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
            power = 0;
        }
        turret->Set(ControlMode::PercentOutput, power);
    }
}


void Shoot::FindTarget(double robot_yaw){
    double turret_rot = turret->GetSelectedSensorPosition();
    //Find the turret location in comparison to robot location and find which direction to turn
}


double Shoot::GetLimelightY(){
    return limelight->getYOff();
}

double Shoot::GetLimelightX(){
    return limelight->getXOff();
}

void Shoot::Auto(){
    
}

//TalonFX integrated encoder: Units per rot = 2048
void Shoot::Zero(){
    turret->SetSelectedSensorPosition(0);
    turret->Set(ControlMode::PercentOutput, 0);
    //Please work
    while(turret->GetSelectedSensorPosition() > -9500){
        turret->Set(ControlMode::PercentOutput, -0.1);
    }
    turret->Set(ControlMode::PercentOutput, 0);
}


void Shoot::Manual_Zero(){
    while(turret_limit_switch->Get()){
        turret->Set(ControlMode::PercentOutput, 0.12);
    }
    turret->Set(ControlMode::PercentOutput, 0);
    turret->SetSelectedSensorPosition(0);
}


//Operator movement
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


//Testing function for calibrating shooter values
void Shoot::Shooter_Calibrate(){
    READING_SDB_NUMERIC(double, Hood_angle_out) hood_out;
    READING_SDB_NUMERIC(double, Flywheel_percent_out) flywheel_out;
    shoot_master->Set(ControlMode::Velocity, flywheel_out);
    shoot_slave->Set(ControlMode::Velocity, -flywheel_out);
    std::cout << "set velocity: " << flywheel_out << std::endl;
    std::cout << "curr velocity: " << shoot_master->GetSelectedSensorVelocity() << std::endl;
    std::cout << "error: " << shoot_master->GetClosedLoopError() << std::endl;
    servo_left.Set(hood_out);
    servo_right.Set(hood_out);
}

//Testing function for calibrating turret values
void Shoot::Turret_Calibrate(){

    //This is to test whether or not the limelight sees the target
    if(limelight->target_valid()){
        std::cout <<"sees the target" << std::endl; 
    } else {
        std::cout << "Does not see the target" << std::endl;
    } 

    READING_SDB_NUMERIC(double, Turret_P) TKp;
    READING_SDB_NUMERIC(double, Turret_I) TKi;
    READING_SDB_NUMERIC(double, Turret_D) TKd;
    x_off = limelight->getXOff();
    double power;
    if (x_off > 500) {
        power = 0;
    }

   double delta_xoff = (x_off - prev_xoff);
   acc_error += x_off;

	double heading_error = x_off;
	power = 0.0;
	power = TKp*heading_error + TKi*acc_error + TKd*delta_xoff;
    prev_xoff = x_off;
    
    if (power < -0.25) power = -0.25;
    if (power > 0.25) power = 0.25;
    if((turret->GetSelectedSensorPosition() > 0 ) || (turret->GetSelectedSensorPosition() < -19870)){
        power = 0;
    }
    turret->Set(ControlMode::PercentOutput, power);
}