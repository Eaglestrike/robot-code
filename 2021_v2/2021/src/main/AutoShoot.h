#include <math.h>
#include <map>
#include <memory>
#include <iostream>

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

class auto_shoot {
    public:

         struct Settings {
            double flywheel_speed;
            double kicker_speed;
            double hood_hangle;
        };

        auto_shoot(Limelight& l);
        auto_shoot::Settings distance_to_settings(double dist);
        double distance(double y_off);
        auto_shoot::Settings auto_shoot_calc(std::shared_ptr<nt::NetworkTable> network_table);
};
