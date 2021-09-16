#pragma once

#include <iostream>
#include <frc/XboxController.h>
#include <frc/Joystick.h>
#include "Drive.h"
#include "Shoot.h"

class Controls{
    public:
        //Function is for periodic at competition
        void Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy);
        
        /*Manual function will be for the operator to move turret, shooter, and drive at will*/
        void Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy);
    
    private:
        Drive _drivetrain;
        Shoot _shooter;

};