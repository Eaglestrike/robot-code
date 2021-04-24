#include "auto_shoot.h"

#define PI 3.14159265
#define INF pow(10, 10); //why not?

std::map<double, std::pair<double, double>> distance_to_settings; 

double distance(double y_off) {// x (horizontal) distance to goal

	double shooter_height = 3.10; //310 cm tall
	double cam_height = 0.55; //taken from config.cc
//	double cam_angle = 8.0; //in degrees, taken from config.cc
	double angle_to_top = y_off; //in degrees???

	//trig ratios, get adjacent side
	return (shooter_height - cam_height) / tan ( (cam_height + angle_to_top) * PI / 180.0);
}


//.first is hood angle, .second is flywheel speed
std::pair<double, double> auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table) {
    double y_off = network_table->GetNumber("ty", 0.0);
	return distance_to_settings[distance(y_off)];
}

//unfinished
/*void center(std::shared_ptr<nt::NetworkTable> network_table) { //makes robot face target
	double Kp = -0.1; //idk its just what the docs had
	double min_command = 0.05;

	double x_off = network_table->GetNumber("tx", 0.0);
//	double y_off = network_table->GetNumber("ty", 0.0);
//	double area = network_table->GetNumber("ta", 0.0);

	//auto position robot so x_off is reasonable
	//https://docs.limelightvision.io/en/latest/cs_aiming.html
	double heading_error = -x_off;
	double steering_adjust = 0.0;
	if (x_off > 1.0) steering_adjust = Kp*heading_error - min_command;
	else if (x_off < 1.0) steering_adjust = Kp*heading_error - min_command;
	//left wheel += steering_adjust
	//right wheel -= steering_adjust
} */



/*POSSIBLE THINGS TO LOOK FOR IN DEBUG:
	- something is in degrees/radians and I thought otherwise
	- include issues (network table, limelight)
	- hardware (limelight, motors, etc) being fucked in general
	- 
	- 
*/