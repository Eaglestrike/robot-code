#include "Intake.h"

Intake::Intake(){
    intake_motor->SetExpiration(30);
}

void Intake::Periodic(){
    switch (state){
        case State::Idle:
            //deployed = false;
            l_pneumatic.Set(false);
            r_pneumatic.Set(false);
            test1_pneumatic.Set(false);
            test2_pneumatic.Set(false);
            intake_motor->Set(ControlMode::PercentOutput, 0);
            break;
        case State::Deploy:
            //deployed = true;
            test1_pneumatic.Set(true);
            test2_pneumatic.Set(true);
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