#pragma once
#include <iostream>
#include "subsystems/SwerveModule.h"
#include <frc/kinematics/SwerveDriveKinematics.h>
#include <frc/kinematics/SwerveDriveOdometry.h>
#include <frc/geometry/Translation2d.h>
#include <frc/geometry/Rotation2d.h>
#include <frc/trajectory/Trajectory.h>
#include <frc/geometry/Pose2d.h>
#include <frc/controller/HolonomicDriveController.h>
#include <frc/controller/PIDController.h>
#include <frc/controller/ProfiledPIDController.h>
#include <AHRS.h>
#include <frc/smartdashboard/SmartDashboard.h>

using namespace DriveConstants;

class Drivetrain{
    public:
        Drivetrain();
        void Drive(units::meters_per_second_t xSpeed,
            units::meters_per_second_t ySpeed, units::radians_per_second_t rot, 
            bool fieldRelative);
        void UpdateOdometry();

        //Maybe only pass in rotation 2d
        void GetRotation(frc::Rotation2d rot);
        void ResetOdometry(const frc::Pose2d& pose);
        frc::Pose2d GetPose();
        void FollowPath();
        void GetEncoderValues();
        void SetPID();
        
        static constexpr units::meters_per_second_t MaxSpeed = 3.0_mps;
        static constexpr units::radians_per_second_t MaxAngularSpeed{3.14};

    private:
        frc::Translation2d frontLeftLocation{0.254_m, 0.2438_m};
        frc::Translation2d frontRightLocation{+0.254_m, -0.2438_m};
        frc::Translation2d backLeftLocation{-0.254_m, +0.2438_m};
        frc::Translation2d backRightLocation{-0.254_m, -0.2438_m};

        SwerveModule front_left{front_left_speedMotor, front_left_angleMotor,
            front_left_angleEncoderA, front_left_angleEncoderB};
        SwerveModule front_right{front_right_speedMotor, front_right_angleMotor,
            front_right_angleEncoderA, front_right_angleEncoderB};
        SwerveModule back_left{back_left_speedMotor, back_left_angleMotor,
            back_left_angleEncoderA, back_left_angleEncoderB};
        SwerveModule back_right{back_right_speedMotor, back_right_angleMotor,
            back_right_angleEncoderA, back_right_angleEncoderB};
        
        frc::SwerveDriveKinematics<4> m_kinematics{
            frontLeftLocation, frontRightLocation, 
            backLeftLocation, backRightLocation};

        //Figure out a way to add navx yaw to rotation2d object
        frc::Rotation2d rotation;

        frc::SwerveDriveOdometry<4> m_odometry{m_kinematics, rotation};

        //For Trajectory following --- testing comment out before deploying
        // frc2::PIDController xController{1, 0, 0};
        // frc2::PIDController yController{1, 0, 0};
        // static constexpr auto ModuleMaxAngularVelocity = 3.1415 * 1_rad_per_s;
        // static constexpr auto ModuleMaxAngularAcceleration = 3.1415 * 2_rad_per_s/1_s;
        // frc::ProfiledPIDController<units::radians> thetaController{
        //     1.0, 0.0, 0.0, {ModuleMaxAngularVelocity, ModuleMaxAngularAcceleration}};
        // frc::HolonomicDriveController swerveController {xController, yController, thetaController};

        //const auto goal = ;
        
};