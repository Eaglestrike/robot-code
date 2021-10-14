#pragma once

#include "ctre/Phoenix.h"
#include "frc/WPILib.h"
#include <frc/Joystick.h>
#include <frc/SpeedControllerGroup.h>
#include <frc/controller/PIDController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>
#include <frc/drive/DifferentialDrive.h>
#include "Controls.h"
#include <frc/SPI.h>
#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>
#include <frc2/Timer.h>
#include <units/units.h>
#include "iostream"

class Drive 
{
    public:
        Drive();
        void periodic(Controls& controls);
        void autoDrive();
        void drive(double throttle, double turn);
        void stop();

    private:
        WPI_TalonFX &leftM = *new WPI_TalonFX(22);
        WPI_TalonFX &leftS = *new WPI_TalonFX(21);
        WPI_TalonFX &rightM = *new WPI_TalonFX(23);
        WPI_TalonFX &rightS = *new WPI_TalonFX(24);
        /*
        25 - left flywheel
        26 - right flywheel

        2 - kicker
        52 - hood
        20 - intake
        31 - wheel thing (not connected)
        42 - connected to nothing (farthest towards the intake/right)
        43 - serializer front (motor not connected)
        44 - second section of serializer (motor not connected)
        19 - connected to nothing

        */

        frc::SpeedControllerGroup leftGroup{leftM, leftS};
        frc::SpeedControllerGroup rightGroup{rightM, rightS};
        frc::DifferentialDrive driveSet{leftGroup, rightGroup};
        
};
