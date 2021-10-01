#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/DigitalInput.h>

class Channel{
    public:
        enum State {
            Idle,
            Intake,
            Shooting
        };

        Channel();
        void Periodic();
        void Stop();
        void setState(State newState);
    
    private:
        State state;

        WPI_TalonFX * channel = new WPI_TalonFX(8);
        WPI_TalonFX * kicker = new WPI_TalonFX(7);
        frc::DigitalInput * photogate = new frc::DigitalInput(2);
};