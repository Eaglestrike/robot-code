#pragma once

#include "ctre/Phoenix.h"
//#include "frc/WPILib.h"
#include <frc/Joystick.h>
#include <frc/XboxController.h>
#include <frc/SpeedControllerGroup.h>
#include <frc/controller/PIDController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>
#include <frc/drive/DifferentialDrive.h>
#include <frc/MotorSafety.h>
#include <cmath>
#include <iostream>

class Drive{
    public:

        Drive();
        void Periodic(double forward, double turn);
        void Auto();
        void Stop();
        void Auto2();

    private:
        WPI_TalonFX * left_master = new WPI_TalonFX(2);
        WPI_TalonFX * left_slave = new WPI_TalonFX(3);
        WPI_TalonFX * right_master = new WPI_TalonFX(0);
        WPI_TalonFX * right_slave = new WPI_TalonFX(1);
        
        frc::SpeedControllerGroup left_side{*left_master, *left_slave};
        frc::SpeedControllerGroup right_side{*right_master, *right_slave};
        frc::DifferentialDrive m_drive{left_side, right_side};
};