#include "Drive.h"
#include <algorithm>

Drive::Drive(){
    
    left_master->SetInverted(TalonFXInvertType::CounterClockwise);
    right_master->SetInverted(TalonFXInvertType::Clockwise);
    left_slave->SetInverted(TalonFXInvertType::FollowMaster);
    right_slave->SetInverted(TalonFXInvertType::Clockwise);

    left_slave->Follow(*left_master);
    right_slave->Follow(*right_master);
}

void Drive::Periodic(double forward, double turn){

    if(abs(forward) < 0.05 && abs(turn) < 0.05){
        //left_master->SetNeutralMode(NeutralMode::Brake);
        //right_master->SetNeutralMode(NeutralMode::Brake);
        //left_slave->SetNeutralMode(NeutralMode::Brake);
        //right_slave->SetNeutralMode(NeutralMode::Brake);
        forward = 0; 
        turn = 0;
    }
    else{
        //left_master->SetNeutralMode(NeutralMode::Coast);
        //right_master->SetNeutralMode(NeutralMode::Coast);
        //left_slave->SetNeutralMode(NeutralMode::Coast);
        //right_slave->SetNeutralMode(NeutralMode::Coast);
    }
    m_drive.ArcadeDrive(forward, turn, false);
    m_drive.SetRightSideInverted(false);
}

void Drive::Auto(){
    m_drive.ArcadeDrive(-0.1, 0, false);
    m_drive.SetRightSideInverted(false);
}

//this came from josh's code, idk what the heck i'm setting, does this require open loop to be configured for the motors?
void Drive::SetWantRawOpenLoop(const frc::DifferentialDriveWheelSpeeds& openloop) {
    left_master->Set(ControlMode::PercentOutput, openloop.left.to<double>()); 
    right_master->Set(ControlMode::PercentOutput, openloop.right.to<double>()); 
}

/**
 * Taken from 254, it changes the controls and feel of manipulating the drive to that of a curvature drive
 * https://www.reddit.com/r/FRC/comments/80679m/what_is_curvature_drive_cheesy_drive/  
**/

frc::DifferentialDriveKinematics kinematics_(units::meter_t(0.7239));

void Drive::SetWantCheesyDrive(double throttle, double wheel, bool quick_turn) {
    throttle = std::min(throttle, 0.05); //i THINK this accomplishes the job? used to be josh's Deadband function
    wheel = std::min(wheel, 0.035);

    constexpr double kWheelGain = 1.97;
    constexpr double kWheelNonlinearity = 0.05;
    const double denominator = std::sin(M_PI / 2.0 * kWheelNonlinearity);
    // Apply a sin function that's scaled to make it feel better.
    if (!quick_turn) {
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = wheel / (denominator * denominator) * std::abs(throttle);
    }
    wheel *= kWheelGain;
    frc::DifferentialDriveWheelSpeeds signal = kinematics_.ToWheelSpeeds(
        frc::ChassisSpeeds{units::meters_per_second_t{throttle}, 0.0_mps,
                           units::radians_per_second_t{wheel}});
    signal.Normalize(1.0_mps);
    SetWantRawOpenLoop(signal);
}