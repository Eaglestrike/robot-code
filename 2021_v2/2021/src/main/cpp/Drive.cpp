#include "Drive.h"

Drive::Drive(){
    left_master->SetInverted(TalonFXInvertType::CounterClockwise);
    right_master->SetInverted(TalonFXInvertType::Clockwise);
    left_slave->SetInverted(TalonFXInvertType::FollowMaster);
    right_slave->SetInverted(TalonFXInvertType::Clockwise);

    left_slave->Follow(*left_master);
    right_slave->Follow(*right_master);

    left_master->SetSafetyEnabled(false);
    right_master->SetSafetyEnabled(false);
    left_slave->SetSafetyEnabled(false);
    right_slave->SetSafetyEnabled(false);
    left_master->SetExpiration(30);
    right_master->SetExpiration(30);
    left_slave->SetExpiration(30);
    right_slave->SetExpiration(30);
    
}

void Drive::Periodic(double forward, double turn){
    left_master->SetNeutralMode(NeutralMode::Brake);
    right_master->SetNeutralMode(NeutralMode::Brake);
    left_slave->SetNeutralMode(NeutralMode::Brake);
    right_slave->SetNeutralMode(NeutralMode::Brake);

    m_drive.ArcadeDrive(forward, turn, false);
    m_drive.SetRightSideInverted(false);
}

void Drive::Auto(){
    m_drive.ArcadeDrive(0.2, 0.0, false);
    m_drive.SetRightSideInverted(false);
}

void Drive::Auto2(){
    m_drive.ArcadeDrive(0.0,0.3, false);
}

void Drive::Stop(){
    m_drive.ArcadeDrive(0,0,false);
}