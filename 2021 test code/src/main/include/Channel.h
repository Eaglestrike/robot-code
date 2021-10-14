#pragma once

#include "ctre/Phoenix.h"
#include "frc/WPILib.h"
#include <frc/Joystick.h>

#include "Controls.h"

class Channel
{
    public:
        Channel();
        void periodic(Controls &controls);
        void run(Controls &controls);

    private:
        WPI_TalonSRX &frontMotor = *new WPI_TalonSRX(43);
        WPI_TalonSRX &sideMotor = *new WPI_TalonSRX(44);

};