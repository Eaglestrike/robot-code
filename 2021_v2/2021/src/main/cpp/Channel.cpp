#include "Channel.h"

Channel::Channel(){
    channel->SetExpiration(10);
    kicker->SetExpiration(10);
}

void Channel::Periodic(){
    switch(state){
        case State::Idle:
            Stop();
           // std::cout<<"IDLE\n";
            break;
        case State::Intake:
          //  std::cout<<"INTAKE\n";
            if(!photogate->Get()){
                Stop();
                return;
            } else {
                channel->Set(ControlMode::PercentOutput, -0.3);
                kicker->Set(ControlMode::PercentOutput, -0.27);
            }
            break;
        case State::Shooting:
         //   std::cout<<"Shooting\n";
            channel->Set(ControlMode::PercentOutput, -0.35);
            kicker->Set(ControlMode::PercentOutput, -0.45);
            break;
        default:
            break;
    }
}

Channel::State Channel::getState() {
    return state;
}

void Channel::setState(Channel::State newState){
    state = newState;
}

void Channel::Stop(){
    kicker->Set(ControlMode::PercentOutput, 0.0);
    channel->Set(ControlMode::PercentOutput, 0.0);
}