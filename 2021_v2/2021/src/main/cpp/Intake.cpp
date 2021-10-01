#include "Intake.h"

Intake::Intake(){
    intake_motor->SetExpiration(5);
}

void Intake::Periodic(){
    switch (state){
        case State::Idle:
            deployed = false;
            if(deployed){
                return;
            }
            l_pneumatic.Set(false);
            r_pneumatic.Set(false);
            intake_motor->Set(ControlMode::PercentOutput, 0);
            break;
        case State::Deploy:
            deployed = true;
            l_pneumatic.Set(true);
            r_pneumatic.Set(true);
            Run();
            break;
        case State::Unjam:
            intake_motor->Set(ControlMode::PercentOutput, -0.25);
            break;
        default:
            break;
    }
}

void Intake::setState(Intake::State newState){
    state = newState;
}


void Intake::Run(){
    intake_motor->Set(ControlMode::PercentOutput, 0.30);
}

bool Intake::Deployed(){
    return deployed;
}