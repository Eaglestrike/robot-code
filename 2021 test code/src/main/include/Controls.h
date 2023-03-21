#pragma once
#include <frc/Joystick.h>
#include "iostream"

class Controls 
{
    public:
        enum controlMethod
        {
            joysticks = 0,
            gamecubeController = 1
        };

        Controls(controlMethod method);

        double throttle();
        double turn();

        bool readyShot();
        bool shoot();

        int intake();
        int deploy();

        bool spinWheel();

    private:
        controlMethod method;
        frc::Joystick gamecube;
        frc::Joystick lJoy;
        frc::Joystick rJoy;

};
