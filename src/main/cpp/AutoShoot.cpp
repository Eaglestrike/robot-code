#include "AutoShoot.h"
#include "Limelight.h"

#define PI 3.14159265
#define INF pow(10, 10); //why not?

double y_off = 0;
double max_dist = 7.42; //last key in map


//distance in meters
std::map<double, AutoShoot::Settings> distance_to_settings_map; 

AutoShoot::AutoShoot() {
	distance_to_settings_map.insert({ 3.28 , (Settings){25.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 3.86 , (Settings){22.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 4.67 , (Settings){20.0, 35000.0, 0.7} });
	distance_to_settings_map.insert({ 5.91 , (Settings){20.0, 40000.0, 0.7} });
	distance_to_settings_map.insert({ 6.24 , (Settings){20.0, 37000.0, 0.75} });
	distance_to_settings_map.insert({ 7.42 , (Settings){20.0, 40000.0, 0.8} });
}

AutoShoot::Settings distance_to_settings(double dist) {
	AutoShoot::Settings best_combo;
	std::map<double, AutoShoot::Settings>::iterator it;
	it = distance_to_settings_map.begin();
	for (it = distance_to_settings_map.begin(); it != distance_to_settings_map.end(); it++) //iterate through settings
	{
		double prevDist = it->first;
		double nextDist = (std::next(it, 1))->first;
		AutoShoot::Settings prevSetting = it->second;
		AutoShoot::Settings nextSetting = (std::next(it, 1))->second;
        if (dist < prevDist) return prevSetting; //min threshold
		if (prevDist <= dist && dist <= nextDist) { //we want something between these settings
			//it's the previous setting plus the difference to the next setting times the percent (0-1) to the next setting the input distance is
			best_combo.flywheel_speed = prevSetting.flywheel_speed + (nextSetting.flywheel_speed - prevSetting.flywheel_speed)*((dist-prevDist)/(nextDist-prevDist));
			best_combo.kicker_speed = prevSetting.kicker_speed + (nextSetting.kicker_speed - prevSetting.kicker_speed)*((dist-prevDist)/(nextDist-prevDist));
			best_combo.hood_angle = prevSetting.hood_angle + (nextSetting.hood_angle - prevSetting.hood_angle)*((dist-prevDist)/(nextDist-prevDist));
            return best_combo;
        }
	}
	return distance_to_settings_map[max_dist];
}

double distance(Limelight& limelight) {// x (horizontal) distance to goal

	double shooter_height = 2.28; //2.65 
	double cam_height = limelight.height;
	double cam_angle = limelight.angle_above_horizontal; 
	double angle_to_top = y_off;

	//trig ratios, get adjacent side
	return (shooter_height - cam_height) / tan ( (cam_angle + angle_to_top) * PI / 180.0);
}

//thinks its 11-12 meters away when it's actually 5-6 meters away
//.first is hood angle, .second is flywheel speed
AutoShoot::Settings AutoShootCalc(std::shared_ptr<nt::NetworkTable> network_table, Limelight& limelight) {
    if (limelight.getYOff() > 0) y_off = limelight.getYOff();
	else std::cout << "doesn't see it" << std::endl;
	return distance_to_settings(distance(limelight));
}