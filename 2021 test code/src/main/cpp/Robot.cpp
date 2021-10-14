#include "Robot.h"
#include <iostream>
#include <frc/smartdashboard/SmartDashboard.h>

/**
 * nothing, mostly just use constructor instead
 */
void Robot::RobotInit() {}

/**
 * initiate robot, call subsystems' constructors
 */
Robot::Robot() : frc::TimedRobot{}, timer{}, controls{Controls::controlMethod::gamecubeController}, drive{}, intake{}, shoot{}
{

}

/**
 * This function is called every robot packet, no matter the mode. Use
 * this for items like diagnostics that you want ran during disabled,
 * autonomous, teleoperated and test.
 *
 * <p> This runs after the mode specific periodic functions, but before
 * LiveWindow and SmartDashboard integrated updating.
 */
void Robot::RobotPeriodic() 
{

}

/**
 * This autonomous (along with the chooser code above) shows how to select
 * between different autonomous modes using the dashboard. The sendable chooser
 * code works with the Java SmartDashboard. If you prefer the LabVIEW Dashboard,
 * remove all of the chooser code and uncomment the GetString line to get the
 * auto name from the text box below the Gyro.
 *
 * You can add additional auto modes by adding additional comparisons to the
 * if-else structure below with additional strings. If using the SendableChooser
 * make sure to add them to the chooser code above as well.
 */
void Robot::AutonomousInit() 
{
    timer.Reset();
    timer.Start();
}
void Robot::AutonomousPeriodic() 
{
    if (timer.Get() < 1)
    {
        drive.autoDrive();
    }else
    {
        drive.stop();
    }
    
}

void Robot::TeleopInit() {}
/**
 * call subsystem periodics, probably other stuff later on
 */
void Robot::TeleopPeriodic() //I do like using a controls class, but passing it each time feels not right, is there a way to make like a global controls class
{
    timer.Stop();
    drive.periodic(controls);
    intake.periodic(controls);
    channel.periodic(controls);

    Shoot::state state = shoot.periodic(controls);
    switch(state)
    {
        case Shoot::idle:
            shoot.stop();
            break;
        case Shoot::aiming: //something about overriding drive?
            drive.drive(0, shoot.horizontalAim());
            //something with vertical hood
            break;
        case Shoot::shooting:
            shoot.shootShot();
            break;
    }
    
}

/**
 * probably nothing, disabled shouldn't do anything
 */
void Robot::DisabledInit() {}
/**
 * probably nothing, don't want anybody dying when it's disabled
 */
void Robot::DisabledPeriodic() {}

/**
 * nothing, just test with main ones
 */
void Robot::TestInit() {}
/**
 * nothing, just test with main ones
 */
void Robot::TestPeriodic() {}

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<Robot>(); }
#endif

