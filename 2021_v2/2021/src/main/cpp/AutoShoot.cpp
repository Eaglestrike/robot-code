#include "AutoShoot.h"
#include "Limelight.h"

#define PI 3.14159265
#define INF pow(10, 10); //why not?

double y_off = 0;
double max_yoff = 22.4; //last key in mapdist


//distance in meters
std::map<double, AutoShoot::Settings> yoff_to_settings_map; 

AutoShoot::AutoShoot() {
	yoff_to_settings_map.insert({ 22.4 , (Settings){0.53, 0, -0.22} });
	yoff_to_settings_map.insert({ 20.1 , (Settings){0.57, 0.05, -0.22} });
	yoff_to_settings_map.insert({ 18.2 , (Settings){0.58, 0.06, -0.22} });
	yoff_to_settings_map.insert({ 14.06 , (Settings){0.59, 0.32, -0.22} });
	yoff_to_settings_map.insert({ 9.5 , (Settings){0.62, 0.35, -0.22} });
	yoff_to_settings_map.insert({ 5.47 , (Settings){0.65, 0.37, -0.22} });
    yoff_to_settings_map.insert({ 4.57 , (Settings){0.68, 0.38, -0.22} });
    yoff_to_settings_map.insert({ 3 , (Settings){0.7, 0.39, -0.22} });
    yoff_to_settings_map.insert({ 1 , (Settings){0.72, 0.39, -0.22} });
    yoff_to_settings_map.insert({ -1.2 , (Settings){0.77, 0.37, -0.22} });
    yoff_to_settings_map.insert({ -3.8 , (Settings){0.79, 0.36  , -0.22} });
}

AutoShoot::Settings yoff_to_settings(double yoff) {
	AutoShoot::Settings best_combo;
	std::map<double, AutoShoot::Settings>::iterator it;
	it = yoff_to_settings_map.begin();
	for (it = yoff_to_settings_map.begin(); it != yoff_to_settings_map.end(); it++) //iterate through settings
	{
		double prevYOff = it->first;
		double nextYOff = (std::next(it, 1))->first;
		AutoShoot::Settings prevSetting = it->second;
		AutoShoot::Settings nextSetting = (std::next(it, 1))->second;
        if (yoff < prevYOff) return prevSetting; //min threshold
		if (prevYOff <= yoff && yoff <= nextYOff) { //we want something between these settings
			//it's the previous setting plus the difference to the next setting times the percent (0-1) to the next setting the input distance is
			best_combo.flywheel_out = prevSetting.flywheel_out + (nextSetting.flywheel_out - prevSetting.flywheel_out)*((yoff-prevYOff)/(nextYOff-prevYOff));
			best_combo.kicker_out = prevSetting.kicker_out + (nextSetting.kicker_out - prevSetting.kicker_out)*((yoff-prevYOff)/(nextYOff-prevYOff));
			best_combo.hood_out = prevSetting.hood_out + (nextSetting.hood_out - prevSetting.hood_out)*((yoff-prevYOff)/(nextYOff-prevYOff));
            return best_combo;
        }
	}
	return yoff_to_settings_map[max_yoff];
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
//	else std::cout << "doesn't see it" << std::endl;
	return yoff_to_settings(distance(limelight));
}