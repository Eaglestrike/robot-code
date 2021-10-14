#include "Drive.h"

/**
 * sets direction and slave master relationships
 */
Drive::Drive()
{
    leftM.SetInverted(TalonFXInvertType::CounterClockwise);
    rightM.SetInverted(TalonFXInvertType::CounterClockwise);
    leftS.SetInverted(TalonFXInvertType::FollowMaster);
    rightS.SetInverted(TalonFXInvertType::CounterClockwise);

    leftS.Follow(leftM);
    rightS.Follow(rightM);
}

/**
 * sets neutral mode and drives the robot
 */
void Drive::periodic(Controls& controls)
{
    leftM.SetNeutralMode(Brake);
    leftS.SetNeutralMode(Brake);
    rightM.SetNeutralMode(Brake);
    rightS.SetNeutralMode(Brake);
    driveSet.ArcadeDrive(controls.throttle(), controls.turn(), false);
    
}

void Drive::drive(double throttle, double turn) { driveSet.ArcadeDrive(throttle, turn, false); }
void Drive::autoDrive() { driveSet.ArcadeDrive(0.2, 0.0, false); }
void Drive::stop() { driveSet.ArcadeDrive(0, 0, false); }