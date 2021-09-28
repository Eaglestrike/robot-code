#include "Channel.h"

Channel::Channel(){
    //Whatever 
}

void Channel::Run(){
    if(photogate->Get()){
        Stop();
        return;
    } else {
    channel->Set(ControlMode::PercentOutput, -0.3);
    kicker->Set(ControlMode::PercentOutput, -0.3);
    }
}

void Channel::Stop(){
    kicker->Set(ControlMode::PercentOutput, 0.0);
    channel->Set(ControlMode::PercentOutput, 0.0);
}

void Channel::kicker_run(double percent_out){
    kicker->Set(ControlMode::PercentOutput, -percent_out);
}