#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/XboxController.h>
#include <cmath>

class Shoot{
    public:
        Shoot();
        void Periodic(const frc::XboxController & xbox);
        void Aim();
        void Auto();
        void Manual(const frc::XboxController & xbox);
        
        enum State {
            Idle,
            Aiming,
            Shooting,
        };
        void setState(State newState);

    private:
        WPI_TalonFX * turret = new WPI_TalonFX(4);
        WPI_TalonFX * shoot_master = new WPI_TalonFX(5);
        WPI_TalonFX * shoot_slave = new WPI_TalonFX(6);
        
        Limelight * limelight = new Limelight();

        State state;

};