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
            Unjam,
            Shoot
        };

        Intake();
        void Run();
        bool Deployed();
        void Periodic();
        void setState(State newState);
        
    private:

        WPI_TalonFX * intake_motor = new WPI_TalonFX(9);
        frc::Solenoid test1_pneumatic{2};
        frc::Solenoid test2_pneumatic{3};
        bool deployed = false;

        State state;
};