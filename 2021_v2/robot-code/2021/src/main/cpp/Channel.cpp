#include "Channel.h"

Channel::Channel(){
    channel_slave->Follow(*channel_master);
}

void Channel::Run(){
    if(photogate->Get()){
        Stop();
        return;
    }
    channel_master->Set(ControlMode::PercentOutput, 0.2);
    channel_slave->Set(ControlMode::PercentOutput, 0.2);
    kicker->Set(ControlMode::PercentOutput, 0.2);
}

void Channel::Stop(){
    kicker->Set(ControlMode::PercentOutput, 0.0);
    channel_master->Set(ControlMode::PercentOutput, 0.0);
    channel_slave->Set(ControlMode::PercentOutput, 0.0);
}