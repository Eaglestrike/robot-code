#include "Channel.h"

Channel::Channel()
{

}

void Channel::periodic(Controls &controls)
{
    run(controls);
}

void Channel::run(Controls &controls)
{
    frontMotor.Set(ControlMode::PercentOutput, controls.intake() * -0.65); //test values
    sideMotor.Set(ControlMode::PercentOutput, controls.intake() * -0.65); //test values, figure out sensor thing
}