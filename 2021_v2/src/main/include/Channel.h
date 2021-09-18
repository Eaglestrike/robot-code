#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"

class Channel{
    public:
        Channel();
        void Run();
        bool Stop();
    
    private:
    //TODO: add photogate
        WPI_TalonFX * channel_master = new WPI_TalonFX(8);
        WPI_TalonFX * channel_slave = new WPI_TalonFX(9);

};