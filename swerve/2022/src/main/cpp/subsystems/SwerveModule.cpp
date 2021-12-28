#include "subsystems/SwerveModule.h"
#include <frc/geometry/Rotation2d.h>


SwerveModule::SwerveModule(int driveMotorPort, int turnMotorPort,
    int angleEncoderPortA, int angleEncoderPortB)
    : driveMotor(driveMotorPort), turnMotor(turnMotorPort),
    turn_Encoder(angleEncoderPortA, angleEncoderPortB) {

    turn_Encoder.SetDistancePerPulse(2 * 3.1415 / EncoderResolution);

    //Limit the controller's input range between -pi and pi & set continouos
    turningPIDcontroller.EnableContinuousInput(-units::radian_t(3.1415), units::radian_t(3.1415));

    driveMotor.SetNeutralMode(NeutralMode::Brake);
    turnMotor.SetNeutralMode(NeutralMode::Brake);
}


frc::SwerveModuleState SwerveModule::GetState(){
    //Return the state of SwerveModuleObject
    return {units::meters_per_second_t{getSpeed()},
        frc::Rotation2d(units::radian_t(turn_Encoder.Get()))};  
}

double SwerveModule::getEncoderValue(){
    std::cout << "turn encoder distance per pulse: "<< turn_Encoder.GetDistancePerPulse() << std::endl;
    return turn_Encoder.Get();
}

void SwerveModule::SetDesiredState(const frc::SwerveModuleState& referenceState){
    
    //Optomize the reference state to avoid spinning more than 90 degrees
    const auto state = frc::SwerveModuleState::Optimize(referenceState,
      units::radian_t(turn_Encoder.Get()));

    //Calculate the drive output from the drive PID controller
    const auto driveOutput = drivePIDController.Calculate(getSpeed(), state.speed.value());

    //Calculate the turn output from the turn PID controller
    const auto turnOutput = turningPIDcontroller.Calculate(units::radian_t(turn_Encoder.Get()), state.angle.Radians());

    const auto driveFeedforward = driveFeedForward.Calculate(state.speed);
    
    const auto turnFeedforward = 0.7*turnFeedForward.Calculate(turningPIDcontroller.GetSetpoint().velocity);

    //Comment out either drive or turn to test each module individually
    driveMotor.SetVoltage(units::volt_t{driveOutput} + driveFeedforward);
    turnMotor.SetVoltage(units::volt_t{turnOutput} + turnFeedforward);
}

double SwerveModule::getSpeed(){
    //find the unit conversion to meters per second and then check gear ratio?
    return driveMotor.GetSelectedSensorVelocity();
}

void SwerveModule::SetPID(){
    double pGain_a = frc::SmartDashboard::GetNumber("P angle", 0.0);
    frc::SmartDashboard::PutNumber("P angle", pGain_a);
    double iGain_a = frc::SmartDashboard::GetNumber("I angle", 0.0);
    frc::SmartDashboard::PutNumber("I angle", iGain_a);
    double dGain_a = frc::SmartDashboard::GetNumber("D angle", 0.0);
    frc::SmartDashboard::PutNumber("D angle", dGain_a);
    turningPIDcontroller.SetPID(pGain_a, iGain_a, dGain_a);
    
    double pGain_v = frc::SmartDashboard::GetNumber("P vel", 0.0);
    frc::SmartDashboard::PutNumber("P vel", pGain_v);
    double iGain_v = frc::SmartDashboard::GetNumber("I vel", 0.0);
    frc::SmartDashboard::PutNumber("I vel", iGain_v);
    double dGain_v = frc::SmartDashboard::GetNumber("D vel", 0.0);
    frc::SmartDashboard::PutNumber("D vel", dGain_v);
    drivePIDController.SetPID(pGain_v, iGain_v, dGain_v);
}