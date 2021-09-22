#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/DigitalInput.h>

class Channel{
    public:
        Channel();
        void Run();
        void Stop();
    
    private:
        WPI_TalonFX * channel_master = new WPI_TalonFX(9);
        WPI_TalonFX * channel_slave = new WPI_TalonFX(10);
        WPI_TalonFX * kicker = new WPI_TalonFX(7);
        frc::DigitalInput *photogate = new frc::DigitalInput(2);
};