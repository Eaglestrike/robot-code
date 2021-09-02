
#include "Drivetrain.h"

Drivetrain::Drivetrain(){
        left_master.SetInverted(false);
        right_master.SetInverted(true);
        left_slave.Follow(left_master);
        right_slave.Follow(right_master);
        left_slave.SetInverted(InvertType::FollowMaster);
        right_slave.SetInverted(InvertType::FollowMaster);
}

void
Drivetrain::Drive(){

}

void
Drivetrain::UpdateOdometry(){

}