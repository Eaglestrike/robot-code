#include "Intake.h"

Intake::Intake(){
    intake_motor->SetSafetyEnabled(false);
}

void Intake::Periodic(){
    switch (state){
        case State::Idle:
            //test1_pneumatic.Set(false);
            test2_pneumatic.Set(false);
            intake_motor->Set(ControlMode::PercentOutput, 0);
            break;
        case State::Deploy:
            //deployed = true;
            //test1_pneumatic.Set(true);
            test2_pneumatic.Set(true);
            Run();
            break;
        case State::Unjam:
            intake_motor->Set(ControlMode::PercentOutput, -0.25);
            break;
        case State::Shoot:
            intake_motor->Set(ControlMode::PercentOutput, 0.25);
            break;
        case State::Climb:
            test2_pneumatic.Set(true);
        default:
            break;
    }
}

void Intake::setState(Intake::State newState){
    state = newState;
}


void Intake::Run(){
    intake_motor->Set(ControlMode::PercentOutput, 0.40);
}

bool Intake::Deployed(){
    return deployed;
}