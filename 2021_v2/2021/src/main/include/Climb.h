#pragma once

#include <ctre/Phoenix.h>
#include <frc/WPILib.h>
#include <frc/XboxController.h>
#include <frc/Joystick.h>
#include <frc/Solenoid.h>

class Climb{
    public:

        Climb();
        void Extend();
        void Retract();
        void Secure();
        void ReExtend();
        void Manual_Climb(double joystick_val);
        bool climbing;

    private:
        WPI_TalonFX * climb_master = new WPI_TalonFX(11);
        WPI_TalonFX * climb_slave = new WPI_TalonFX(12);
        frc::Solenoid climb_1{0};
        frc::Solenoid climb_2{1};
};
