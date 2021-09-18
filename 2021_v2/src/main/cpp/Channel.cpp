#include "Channel.h"

Channel::Channel(){
    channel_slave->Follow(*channel_master);
}

void Channel::Run(){
    channel_master->Set(ControlMode::PercentOutput, 0.2);
    channel_slave->Set(ControlMode::PercentOutput, 0.2);
}

bool Channel::Stop(){
    //Check if photogate is active. If true, stop and return;
}