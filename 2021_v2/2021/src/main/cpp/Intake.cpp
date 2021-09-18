#include "Intake.h"

void Intake::Deploy(){
    deployed = true;
    left_pneumatic.Set(true);
    right_pneumatic.Set(true);
}

void Intake::Retract(){
    if(!deployed){
        return;
    }
    deployed = false;
    left_pneumatic.Set(false);
    right_pneumatic.Set(false);
}

void Intake::Run(){
    intake_motor->Set(ControlMode::PercentOutput, 0.25);
}

void Intake::Unjam(){
    intake_motor->Set(ControlMode::PercentOutput, -0.25);
}

bool Intake::Deployed(){
    return deployed;
}