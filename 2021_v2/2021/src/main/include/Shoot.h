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
#include <vector>
#include <unordered_map>
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
        bool Its_gonna_shoot();
        void ShortShot();
        void Shooter_Calibrate();
        //void output_to_tine_f();
        double GetLimelightY();
        double GetLimelightX();
        bool interpolate(std::vector<double>& array, double p, double& p1, double& p2);
        void Unjam();

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

        std::vector<double> dataPoints = {-3.8, -1.2, 1.0, 3.0, 4.57, 5.47, 9.5, 14.06, 18.2, 20.1, 22.4};

	    // hash map: data points => {angle, speed}
	    std::unordered_map<double, std::pair<double, double>> dataMap;
};