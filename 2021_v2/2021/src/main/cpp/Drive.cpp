#include "Drive.h"

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
