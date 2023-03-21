#include "Limelight.h"

Limelight::Limelight() 
{
    table = nt::NetworkTableInstance::GetDefault().GetTable("limelight");
}

double Limelight::getX() { return table->GetNumber("tx", 100); }
double Limelight::getY() { return table->GetNumber("ty", 100); }

double Limelight::getDistance()
{
    //zero x?
    double angle, y;
    if((y = getY()) == 100)
    {
        return -1;
    }
    angle = y + VERTICAL_ANGLE_OFFSET;
    return GOAL_HEIGHT / tan(angle);
}