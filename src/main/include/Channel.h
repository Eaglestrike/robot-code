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
        void kicker_run(double percent_out);
    
    private:
        WPI_TalonFX * channel = new WPI_TalonFX(8);
        WPI_TalonFX * kicker = new WPI_TalonFX(7);
        frc::DigitalInput * photogate = new frc::DigitalInput(2);
};