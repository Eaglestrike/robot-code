#include "auto_shoot.h"

#define PI 3.14159265
#define INF pow(10, 10); //why not?

//distance in meters
std::map<double, std::pair<double, double>> distance_to_settings_map; 

void auto_shoot_init() {
	distance_to_settings_map.insert({ 2.6162 , {30.0, 30000.0} });
	distance_to_settings_map.insert({ 3.1115 , {26.0, 34000.0} });
	distance_to_settings_map.insert({ 3.8227 , {28.0, 30000.0} });
	distance_to_settings_map.insert({ 4.0259 , {20.0, 35000.0} });
	distance_to_settings_map.insert({ 4.5212 , {25.0, 40000.0} });	
	distance_to_settings_map.insert({ 5.5626 , {20.0, 38000.0} });
	distance_to_settings_map.insert({ 6.6929 , {20.0, 37700.0} });
	distance_to_settings_map.insert({ 7.5946 , {20.0, 40000.0} });
	distance_to_settings_map.insert({ 8.1534 , {20.0, 40000.0} });
	distance_to_settings_map.insert({ 9.5123 , {20.0, 45000.0} });
}

std::pair<double, double> distance_to_settings(double dist) {
	std::pair<double, double> best_combo;
	double best_dist = 0;
	for (auto const& x : distance_to_settings_map) {
		if (abs(x.first-dist) <= abs(best_dist-dist)) {
			best_combo = x.second;
			best_dist = x.first;
		}
	}
	return best_combo;
}

double distance(double y_off) {// x (horizontal) distance to goal

	double shooter_height = 2.41; //310 cm tall
	double cam_height = 0.55; //taken from config.cc
//	double cam_angle = 8.0; //in degrees, taken from config.cc
	double angle_to_top = y_off; //in degrees???

	//trig ratios, get adjacent side
	return (shooter_height - cam_height) / tan ( (cam_height + angle_to_top) * PI / 180.0);
}

//thinks its 11-12 meters away when it's actually 5-6 meters away
//.first is hood angle, .second is flywheel speed
std::pair<double, double> auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table) {
    double y_off = network_table->GetNumber("ty", 0.0);
	std::cout << "distance (in meters): " << distance(y_off) << std::endl;
	std::cout << "flywhel speed: " << distance_to_settings(distance(y_off)).second << " hood angle: " << distance_to_settings(distance(y_off)).first << std::endl;
	return distance_to_settings(distance(y_off));
}



/*POSSIBLE THINGS TO LOOK FOR IN DEBUG:
	- something is in degrees/radians and I thought otherwise
	- include issues (network table, limelight)
	- the motors or the limelight is being stupid
	- 
	- 
*/
