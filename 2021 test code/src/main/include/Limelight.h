#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"
#include <math.h>

class Limelight{
    public:
        Limelight();
        double getX();
        double getY();
        double getDistance();

        const double VERTICAL_ANGLE_OFFSET = 0; //TODO measure values
        const double VERTICAL_HEIGHT_OFFSET = 0; 
        const double GOAL_HEIGHT = 0;

    private:
        std::shared_ptr<nt::NetworkTable> table; //TODO figure out what shared pointer is

};