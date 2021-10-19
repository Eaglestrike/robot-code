#pragma once

#include "ctre/Phoenix.h"
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
        void navx_testing(float yaw);

    private:
        //Motor ids
        const static int l_motor_master_id = 2;
        const static int l_motor_slave_id = 3;
        const static int r_motor_master_id = 0;
        const static int r_motor_slave_id = 1;

        //Drive Motors
        WPI_TalonFX * left_master = new WPI_TalonFX(l_motor_master_id);
        WPI_TalonFX * left_slave = new WPI_TalonFX(l_motor_slave_id);
        WPI_TalonFX * right_master = new WPI_TalonFX(r_motor_master_id);
        WPI_TalonFX * right_slave = new WPI_TalonFX(r_motor_slave_id);
        
        //Drive Base
        frc::SpeedControllerGroup left_side{*left_master, *left_slave};
        frc::SpeedControllerGroup right_side{*right_master, *right_slave};
        frc::DifferentialDrive m_drive{left_side, right_side};

        //PID
        //TODO: 
        double Kp = 0.010, Ki = 0.0, Kd = 0.005;
        double pid_output = 0.0;
        frc2::PIDController m_yaw_controller{Kp, Ki, Kd};
};