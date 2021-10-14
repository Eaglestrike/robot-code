#include "Shoot.h"

Shoot::Shoot() : limelight{}
{
    leftFly.SetInverted(TalonFXInvertType::Clockwise);   //TODO check for legit safety reasons
    rightFly.SetInverted(TalonFXInvertType::CounterClockwise);

    //shootS.Follow(shootM);
}

Shoot::state Shoot::periodic(Controls &controls)
{
    if(controls.readyShot())
    {
        if(controls.shoot())
        {
            currentState = Shoot::shooting;
        }else
        {
            currentState = Shoot::aiming;
        }
    }else
    {
        currentState =  Shoot::idle;
    }

    return currentState;

}

void Shoot::spinUp(double speed)
{

    flywheelGroup.Set(speed); //PID?
}

void Shoot::stop()
{
    flywheelGroup.Set(0);
    kicker.Set(0);
    //something with hood
}

void Shoot::shootShot()
{
    kicker.Set(ControlMode::PercentOutput, 0.45); //TODO test value
}

double Shoot::horizontalAim() //TODO something with not finding a target
{
    double prevError, deltaError;
    
    prevError += hError;
    hError = limelight.getX();
    if( abs(hError) < 0.5 ) //TODO test this value, also is this need if I have a PID? I just don't think I can perfectly fix oscilations
    {
        return 0;
    }
    deltaError = hError - prevError;

    double power = (aimTKp * hError) + (aimTKi * prevError) + (aimTKd * deltaError);

    if (power < -0.85) power = -0.85;
    if (power > 0.85) power = 0.85;

    return power;
}

double Shoot::verticalAim()//TODO very unfinished
{
    double prevError, deltaError;
    
    prevError += vError;
    //vError = something idk 
    if( abs(vError) < 0.5 ) 
    {
        return 0;
    }
    deltaError = vError - prevError;

    double power = (aimTKp * vError) + (aimTKi * prevError) + (aimTKd * deltaError);

    if (power < -0.85) power = -0.85;
    if (power > 0.85) power = 0.85;

    return power;
}