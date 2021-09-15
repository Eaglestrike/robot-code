#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/XboxController.h>
#include "Limelight.h"
#include <frc/Servo.h>
#include <cmath>

class Shoot{
    public:
        enum State {
            Idle,
            Aiming,
            Shooting,
        };

        Shoot();
        void Periodic();
        void Aim();
        void Auto();
        void Manual(const frc::XboxController & xbox);
        void setState(State newState);

    private:
        WPI_TalonFX * turret = new WPI_TalonFX(4);
        WPI_TalonFX * shoot_master = new WPI_TalonFX(5);
        WPI_TalonFX * shoot_slave = new WPI_TalonFX(6);

        frc::Servo servo_left{2};
        frc::Servo servo_right{3};

        Limelight * limelight = new Limelight();

        State state;
};