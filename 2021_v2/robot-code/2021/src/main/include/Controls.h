#pragma once

#include <iostream>
#include <frc/XboxController.h>
#include <frc/Joystick.h>
#include "Drive.h"
#include "Shoot.h"
#include "Intake.h"
#include "Channel.h"
#include "Climb.h"

class Controls{
    public:
        //Function is for periodic at competition
        void Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy);

        //Function is for autonoumous at competition
        void Auto();
        
        //Manual function will be for the operator to move turret, shooter, and drive at will
        void Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy);
    
        //This is for calibration to see values on dashboard
        void Testing(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy);

        void Zero();

    private:
        Drive _drivetrain;
        Shoot _shooter;
        Intake _intake;
        Channel _channel;
        Climb _climb;
};