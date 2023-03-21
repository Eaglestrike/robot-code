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
    }else if (method == controlMethod::joysticks)
    { 
        return lJoy.GetY(); 
    }else 
    {
        return 0;
    }
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
    }else if(method == controlMethod::joysticks)
    { 
        return -rJoy.GetX(); 
    }else 
    {
        return 0;
    }
    
}

bool Controls::readyShot() { return lJoy.GetTrigger(); } //unfished for gamecube
bool Controls::shoot() { return rJoy.GetTrigger(); } //unfished for gamecube

int Controls::intake() 
{ 
    if(method == controlMethod::gamecubeController)
    {
        return gamecube.GetRawButton(1) - gamecube.GetRawButton(2);
    }else if(method == controlMethod::joysticks)
    {
        return lJoy.GetRawButton(3) - rJoy.GetRawButton(4);
    }else 
    {
        return 0;
    }
}
int Controls::deploy() { return lJoy.GetRawButton(2) - rJoy.GetRawButton(2); } //unfished for gamecube, probably don't need

bool Controls::spinWheel() { return rJoy.GetRawButton(4); } //unfished for gamecube

/*

trigger 1 (check numbers)
bottom 2
left 3
right 4
other stuff

*/