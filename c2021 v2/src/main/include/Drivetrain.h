#pragma once

#include <frc/controller/PIDController.h>
#include <units/angle.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>
#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>

#include "ctre/Phoenix.h"

class Drivetrain {
    public:
        Drivetrain();
        void Drive();
        void UpdateOdometry(); 

    private:
        TalonFX left_master = {0};
        TalonFX left_slave = {1};
        TalonFX right_master = {2};
        TalonFX right_slave = {3};

};


