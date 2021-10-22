#include "Climb.h"

Climb::Climb(){
    //Gearbox motors in reverse?
    climbing = false;
    climb_1.Set(true);
    climb_2.Set(true);
}

void Climb::Extend(){
    climb_master->SetNeutralMode(NeutralMode::Coast);
    climb_slave->SetNeutralMode(NeutralMode::Coast);
    climb_1.Set(false);
    climb_2.Set(false);

//Gearbox stuff
}


int hard_stop_ticks = 0; //set once we know
void Climb::Manual_Climb(double joystick_val) { //mostly copied manual turret
    if (!climbing) return; // just in case
    if(abs(joystick_val) <= 0.05 || (joystick_val > 0 && climb_master->GetSelectedSensorPosition() >= hard_stop_ticks)
        || (joystick_val < 0 && climb_master->GetSelectedSensorPosition() <= 0) ){
        joystick_val = 0;
    }
    //commented out part is so climb can go up and down, in place code is only going up
  /*  climb_master->Set(ControlMode::PercentOutput, joystick_val*0.9);
    climb_master->Set(ControlMode::PercentOutput, joystick_val*0.9); */
    climb_master->Set(ControlMode::PercentOutput, std::max(0.0, joystick_val*0.9));
    climb_master->Set(ControlMode::PercentOutput, std::max(0.0, joystick_val*0.9));
}



void Climb::Retract(){
    Secure();
    climb_master->SetSelectedSensorPosition(0);
    climb_slave->SetSelectedSensorPosition(0);
    while (climb_master->GetSelectedSensorPosition() < hard_stop_ticks) { //should this be a state? does while loop cause problems?
        climb_master->Set(ControlMode::PercentOutput, 0.9); //change if bad?
        climb_slave->Set(ControlMode::PercentOutput, 0.9); //change if bad?
    }
}

void Climb::Secure(){
    climb_master->SetNeutralMode(NeutralMode::Brake);
    climb_slave->SetNeutralMode(NeutralMode::Brake);
}

void Climb::ReExtend() {
    while (climb_master->GetSelectedSensorPosition() > 0) { //should this be a state? does while loop cause problems?
        climb_master->Set(ControlMode::PercentOutput, 0.6); //change if bad?
        climb_slave->Set(ControlMode::PercentOutput, 0.6); //change if bad?
    }
}

