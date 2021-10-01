#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include <frc/WPILib.h>
#include <frc/Solenoid.h>


class Intake{
    public:
        enum State {
            Idle,
            Deploy,
            Unjam
        };

        Intake();
        void Run();
        bool Deployed();
        void Periodic();
        void setState(State newState);
        
    private:

        WPI_TalonFX * intake_motor = new WPI_TalonFX(9);
        frc::Solenoid l_pneumatic{0};
        frc::Solenoid r_pneumatic{1};
        bool deployed = false;

        State state;
};