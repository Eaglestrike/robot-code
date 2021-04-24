#include <math.h>
#include <map>
#include <memory>

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

double distance(double y_off);
std::pair<double, double> auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table);
void center(std::shared_ptr<nt::NetworkTable> network_table);