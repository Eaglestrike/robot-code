#include "Intake.h"

Intake::Intake()
{
    
}

/**
 * right no just calls intake function
 */
void Intake::periodic(Controls &controls)
{
    intake(controls);
    
    /*switch(state)
    {
        case State::Intaking:
            intake();
            break;
        case State::Unjamming:
            unjam();
            break;
        case State::Idle:
            intakeMotor.Set(ControlMode::PercentOutput, 0);
            break;
    }*/
}

/**
 * sets the desired state for the intake
 */
void Intake::setState(Intake::State setState)
{
    state = setState;
}

/**
 * sets the intake's motor speed based on control inputs, calls deploy function
 */
void Intake::intake(Controls &controls)
{
    if(controls.intake() != 0)
    {
        deploy();
    }else
    {
        retract();
    }
    intakeMotor.Set(ControlMode::PercentOutput, controls.intake() * 0.65); //test powers
}

/**
 * there is no motor or pneumatic to deploy the intake right now
 */
void Intake::deploy()
{
    if(isDeployed()/* || controls.deploy() == 0*/)
        return;
    //do something with pneumatics or motors
    deployed = true;

    
}
void Intake::retract()
{
    if(!isDeployed())
        return;
    //do something with pneumatics or motors
    deployed = false;
}