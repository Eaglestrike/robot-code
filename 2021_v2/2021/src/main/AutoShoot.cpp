#include "auto_shoot.h"
#include "Limelight.h"

#define PI 3.14159265
#define INF pow(10, 10); //why not?

double y_off = 0;
Limelight& limelight;

//distance in meters
std::map<double, auto_shoot::Settings> distance_to_settings_map; 

auto_shoot::auto_shoot(Limelight& l) {
	limelight = l;
	distance_to_settings_map.insert({ 3.28 , {25.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 3.86 , {22.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 4.67 , {20.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 5.91 , {20.0, 40000.0, 0.7} });
	distance_to_settings_map.insert({ 6.24 , {20.0, 37000.0, 0.75} });
	distance_to_settings_map.insert({ 7.42 , {20.0, 40000.0, 0.8} });
}

//replace with interpolation code?
auto_shoot::Settings distance_to_settings(double dist) {
	auto_shoot::Settings best_combo;
	double best_dist = 0;
	for (auto x : distance_to_settings_map) {
		if (abs(x.first-dist) <= abs(best_dist-dist)) {
			best_dist = x.first;
			best_combo = x.second;
		}
	}
	return best_combo;
}

double distance() {// x (horizontal) distance to goal

	double shooter_height = 2.28; //2.65 
	double cam_height = limelight.height;
	double cam_angle = limelight.angle_above_horizontal; 
	double angle_to_top = y_off;

	//trig ratios, get adjacent side
	return (shooter_height - cam_height) / tan ( (cam_angle + angle_to_top) * PI / 180.0);
}

//thinks its 11-12 meters away when it's actually 5-6 meters away
//.first is hood angle, .second is flywheel speed
auto_shoot::Settings auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table) {
    if (limelight.getYOff() > 0) y_off = limelight.getYOff();
	else std::cout << "doesn't see it" << std::endl;
	return distance_to_settings(distance());
}

