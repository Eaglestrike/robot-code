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
#include "AutoShoot.h"
#include "Channel.h"

class Shoot{
    public:
        enum State {
            Idle,
            Aiming,
            Shooting
        };

        Shoot();
        void Periodic();
        void Aim();
        void Auto();
        void setState(State newState);
        void Zero();
        void Manual_Turret(double turret_rot);
        void Its_gonna_shoot();
        void Shooter_Calibrate();
        void output_to_tine_f();
        
        //void Manual(const frc::XboxController & xbox);

    private:

        WPI_TalonFX * turret = new WPI_TalonFX(4);
        WPI_TalonFX * shoot_master = new WPI_TalonFX(5);
        WPI_TalonFX * shoot_slave = new WPI_TalonFX(6);

        frc::Servo servo_left{3};
        frc::Servo servo_right{4};

        Limelight * limelight = new Limelight();

        State state;

        double TKi, TKp, TKd, x_off;
        double flywheel_out, hood_out, kicker_out;

        frc::DigitalInput *turret_limit_switch = new frc::DigitalInput(1);

        void output_to_time_init();
        std::map <double, double> output_to_time;
        double max_out = 0.9; //can adjust 
};