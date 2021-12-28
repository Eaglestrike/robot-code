#include "subsystems/Drivetrain.h"

Drivetrain::Drivetrain(){

}

void Drivetrain::Drive(units::meters_per_second_t xSpeed,
    units::meters_per_second_t ySpeed, units::radians_per_second_t rot, 
    bool fieldRelative) {
    auto states = m_kinematics.ToSwerveModuleStates(
        fieldRelative ? frc::ChassisSpeeds::FromFieldRelativeSpeeds(xSpeed, ySpeed, rot, rotation)
        : frc::ChassisSpeeds{xSpeed, ySpeed, rot});
        
    m_kinematics.NormalizeWheelSpeeds(&states, MaxSpeed);

    auto [fl, fr, bl, br] = states;

    //std::cout << "front left angle: " << fl.angle.Degrees() << std::endl;

    front_left.SetDesiredState(fl);
    front_right.SetDesiredState(fr);
    back_left.SetDesiredState(bl);
    back_right.SetDesiredState(br);
    GetEncoderValues();
}

void Drivetrain::UpdateOdometry() {
    m_odometry.Update(rotation, front_left.GetState(),
                front_right.GetState(), back_left.GetState(),
                back_right.GetState());
}

void Drivetrain::GetRotation(frc::Rotation2d rot){
    rotation = rot * 0.01745;
}

void Drivetrain::ResetOdometry(const frc::Pose2d& pose){
    m_odometry.ResetPosition(pose, rotation);
}

frc::Pose2d Drivetrain::GetPose() {
    return m_odometry.GetPose();
}

void Drivetrain::GetEncoderValues(){
    std::cout << "FL encoder" << front_left.getEncoderValue() << std::endl;
}

void Drivetrain::SetPID(){
    front_left.SetPID();
    front_right.SetPID();
    back_left.SetPID();
    back_right.SetPID();
}