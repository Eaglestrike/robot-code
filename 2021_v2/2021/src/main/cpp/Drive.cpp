#include "Drive.h"

Drive::Drive(){
    
    left_master->SetInverted(TalonFXInvertType::CounterClockwise);
    right_master->SetInverted(TalonFXInvertType::Clockwise);
    left_slave->SetInverted(TalonFXInvertType::FollowMaster);
    right_slave->SetInverted(TalonFXInvertType::Clockwise);

    left_slave->Follow(*left_master);
    right_slave->Follow(*right_master);
}

void Drive::Periodic(const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    double forward = l_joy.GetRawAxis(1);
    double turn = -1*r_joy.GetRawAxis(0);

    /*if(abs(forward) < 0.05 && abs(turn) < 0.05){
        left_master->SetNeutralMode(Brake);
        right_master->SetNeutralMode(Brake);
        left_slave->SetNeutralMode(Brake);
        right_slave->SetNeutralMode(Brake);
    }
    else{
        left_master->SetNeutralMode(Coast);
        right_master->SetNeutralMode(Coast);
        left_slave->SetNeutralMode(Coast);
        right_slave->SetNeutralMode(Coast);*/
    m_drive.ArcadeDrive(forward, turn, false);
    m_drive.SetRightSideInverted(false);
    
}

void Drive::Auto(){
    m_drive.ArcadeDrive(-0.1, 0, false);
    m_drive.SetRightSideInverted(false);
}
