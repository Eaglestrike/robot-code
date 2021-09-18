#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include <frc/WPILib.h>
#include <frc/Solenoid.h>


class Intake{
    public:
        void Deploy();
        void Run();
        void Retract();
        void Unjam();
        bool Deployed();
        
    private:

        WPI_TalonFX * intake_motor = new WPI_TalonFX(7);
        frc::Solenoid right_pneumatic{0};
        frc::Solenoid left_pneumatic{1};
        bool deployed = false;
};