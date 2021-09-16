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

    m_drive.ArcadeDrive(forward, turn, false);
    m_drive.SetRightSideInverted(false);
}

void Drive::Auto(){
    m_drive.ArcadeDrive(-0.1, 0, false);
    m_drive.SetRightSideInverted(false);
}
