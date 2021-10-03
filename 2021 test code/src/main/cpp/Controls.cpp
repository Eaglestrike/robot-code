#include "Controls.h"

Controls::Controls(controlMethod setMethod) : method{setMethod}, gamecube{setMethod - 1}, lJoy{setMethod * 2}, rJoy{setMethod + 1}
{
    //I know this is a horrible solution but I couldn't find a better one while still
    //using an enum and I'm the only one using the gamecube so it should be fine
}

/**
 * gets the input based on control method for throttle
 */
double Controls::throttle() 
{ 
    if(method == controlMethod::gamecubeController)
    {
        double value = -gamecube.GetRawAxis(1);
        if (value > -0.08 || value < -0.28) 
        { 
            return value; 
        } else {return 0;}
    }else{ return lJoy.GetY(); }
}
/**
 * gets the input based on control method for turn
 */
double Controls::turn() 
{
    if(method == controlMethod::gamecubeController)
    {
        double value = -gamecube.GetRawAxis(0);
        if (value > 0.15 || value < -0.15) 
        { 
            return value; 
        } else {return 0;} 
    }else{ return -rJoy.GetX(); }
    
}

bool Controls::revShooter() { return lJoy.GetTrigger(); } //unfished for gamecube
bool Controls::shoot() { return rJoy.GetTrigger(); } //unfished for gamecube

int Controls::intake() 
{ 
    return lJoy.GetRawButton(3) - rJoy.GetRawButton(2); //unfished for gamecube
}
int Controls::deploy() { return lJoy.GetRawButton(1) - rJoy.GetRawButton(1); } //unfished for gamecube

bool Controls::spinWheel() { return rJoy.GetRawButton(3); } //unfished for gamecube

/*

trigger 0 (check numbers)
bottom 1
left 2
right 3
other stuff

*/