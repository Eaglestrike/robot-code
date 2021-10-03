#pragma once

#include "ctre/Phoenix.h"
#include "frc/WPILib.h"
#include <frc/Joystick.h>

#include "Controls.h"

class Intake
{
    public: 
        enum State //Here for if it is needed in the future, no use for it right now
        {
            Idle,
            Intaking,
            Unjamming
        };

        Intake();
        void periodic(Controls &controls);
        void setState(State state);

        void intake(Controls &controls);
        void deploy(Controls &controls);

    private:
        WPI_TalonSRX &intakeMotor = *new WPI_TalonSRX(20);
        State state;
        bool deployed = false;

};