#include <math.h>
#include <map>
#include <memory>
#include <iostream>

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

class AutoShoot {
    public:
         struct Settings {
            double flywheel_speed;
            double kicker_speed;
            double hood_angle;
        };

        AutoShoot();
        AutoShoot::Settings distance_to_settings(double dist);
        double distance(double y_off);
        AutoShoot::Settings AutoShootCalc(std::shared_ptr<nt::NetworkTable> network_table);
};