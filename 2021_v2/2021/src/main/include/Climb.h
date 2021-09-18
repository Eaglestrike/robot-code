#pragma once

#include <ctre/Phoenix.h>
#include <frc/WPILib.h>

//Want climb on operator or driver?
#include <frc/XboxController.h>
#include <frc/Joystick.h>

class Climb{
    public:
        enum State {
            Hold,
            Climbing
        };

        Climb();
        void Extend();
        void Retract();

        //To put the Falcons in brake mode
        void Secure();
        void setState(State newState);

    private:
        WPI_TalonFX * climb_master = new WPI_TalonFX(10);
        WPI_TalonFX * climb_slave = new WPI_TalonFX(11);

        State state;
};