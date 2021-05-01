#include <math.h>
#include <map>
#include <memory>
#include <iostream>

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

void auto_shoot_init();
std::pair<double, double> distance_to_settings(double dist);
double distance(double y_off);
std::pair<double, double> auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table);
void center(std::shared_ptr<nt::NetworkTable> network_table);
