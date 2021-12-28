#pragma once

#include <frc/Encoder.h>
#include <frc/AnalogEncoder.h>
#include <frc/controller/PIDController.h>
#include <frc/controller/ProfiledPIDController.h>
#include <frc/controller/SimpleMotorFeedforward.h>
#include <ctre/Phoenix.h>
#include <frc/kinematics/SwerveModuleState.h>
#include <units/angular_velocity.h>
#include <units/time.h>
#include <units/voltage.h>
#include <units/velocity.h>

#include "Constants.h"
#include <frc/smartdashboard/SmartDashboard.h>

class SwerveModule {
    public:

        SwerveModule(int driveMotorPort, int turnMotorPort,
            int angleEncoderPortA, int angleEncoderPortB);

        frc::SwerveModuleState GetState();
        void SetDesiredState(const frc::SwerveModuleState& state);
        double getSpeed();
        void SetPID();
        double getEncoderValue();

    private:
        static constexpr double WheelRadius = 0.05;
        static constexpr int EncoderResolution = 4096;

        static constexpr auto ModuleMaxAngularVelocity = 3.1415 * 1_rad_per_s;
        static constexpr auto ModuleMaxAngularAcceleration = 3.1415 * 2_rad_per_s/1_s;

        WPI_TalonFX driveMotor;
        WPI_TalonFX turnMotor;

        frc::Encoder turn_Encoder;

        frc2::PIDController drivePIDController{0, 0, 0};

        frc::ProfiledPIDController<units::radians> turningPIDcontroller{
            0.0, 0.0, 0.0, {ModuleMaxAngularVelocity, ModuleMaxAngularAcceleration}};

        frc::SimpleMotorFeedforward<units::meters> driveFeedForward{1_V, 3_V/1_mps};

        frc::SimpleMotorFeedforward<units::radians> turnFeedForward{1_V, 0.5_V / 1_rad_per_s};

};