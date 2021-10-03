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
Robot::Robot() : frc::TimedRobot{}, controls{Controls::controlMethod::joysticks}, drive{}, intake{}
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
void Robot::AutonomousInit() {}
void Robot::AutonomousPeriodic() {}

void Robot::TeleopInit() {}
/**
 * call subsystem periodics, probably other stuff later on
 */
void Robot::TeleopPeriodic() 
{
    drive.periodic(controls);
    intake.periodic(controls);
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

