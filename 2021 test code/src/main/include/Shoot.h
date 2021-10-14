#pragma once

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/Joystick.h>
#include "ctre/Phoenix.h"
#include "frc/WPILib.h"
#include <math.h>

#include "Limelight.h"
#include "Controls.h"

class Shoot
{
    public:
        enum state
        {
            aiming, shooting, idle
        };
        Shoot();
        state periodic(Controls &controls);
        void spinUp(double speed);
        void shootShot();
        void stop();
        double horizontalAim();
        double verticalAim();

        const double aimTKi = 0.0; //TODO testing at some point
        const double aimTKp = 0.0;
        const double aimTKd = 0.0;
        double hError = 0.0;
        double vError = 0.0;
        
    private:
        Limelight limelight;

        WPI_TalonSRX &kicker = *new WPI_TalonSRX(2);
        WPI_TalonFX &leftFly = *new WPI_TalonFX(25);
        WPI_TalonFX &rightFly = *new WPI_TalonFX(26);

        frc::SpeedControllerGroup flywheelGroup{leftFly, rightFly}; //hopefully they can go different directions
        
        state currentState;
};