#pragma once

#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/XboxController.h>
#include "Limelight.h"
#include <frc/Servo.h>
#include "sdb_types.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/DigitalInput.h>
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
        void Calibration();
        void Zero();

    private:
        WPI_TalonFX * turret = new WPI_TalonFX(4);
        WPI_TalonFX * shoot_master = new WPI_TalonFX(5);
        WPI_TalonFX * shoot_slave = new WPI_TalonFX(6);
        //WPI_TalonFX * kicker = new WPI_TalonFX(7);

        frc::Servo servo_left{2};
        frc::Servo servo_right{3};

        Limelight * limelight = new Limelight();

        State state;

        double TKi, TKp, x_off;
        double flywheel_speed = 0, hood_angle = 0;

        frc::DigitalInput *turret_limit_switch = new frc::DigitalInput(1);
};