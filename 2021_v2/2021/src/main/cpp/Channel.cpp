#include "Channel.h"

Channel::Channel(){
    channel->SetExpiration(5);
    kicker->SetExpiration(5);
}

void Channel::Periodic(){
    switch(state){
        case State::Idle:
            Stop();
        case State::Intake:
            if(!photogate->Get()){
                Stop();
                return;
            } else {
                channel->Set(ControlMode::PercentOutput, -0.3);
                kicker->Set(ControlMode::PercentOutput, -0.27);
            }
            break;
        case State::Shooting:
            channel->Set(ControlMode::PercentOutput, -0.3);
            kicker->Set(ControlMode::PercentOutput, -0.4);
            break;
        default:
            break;
    }
}

void Channel::setState(Channel::State newState){
    state = newState;
}

void Channel::Stop(){
    kicker->Set(ControlMode::PercentOutput, 0.0);
    channel->Set(ControlMode::PercentOutput, 0.0);
}