#include "Robot.h"

#include <units/units.h>

#include <iostream>

namespace team114 {
namespace c2020 {

/**
 * Instentiates structs for future use.
**/
Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      controls_{},
      drive_{Drive::GetInstance()},
      robot_state_{RobotState::GetInstance()},
      ljoy_{0},
      rjoy_{1},
      ojoy_{2},
      cfg{conf::GetConfig()} {}

/**
 * Nothing.
**/
void Robot::RobotInit() {
}


/**
 * Calls period function of select classes. Possibly unfinished?
**/
void Robot::RobotPeriodic() {

    drive_.Periodic();
  
}

/**
 * Zeroing sensors, selecting auto mode.
**/
void Robot::AutonomousInit() {

}

/**
 * Calls periodic function of select structs.
**/
void Robot::AutonomousPeriodic() {

}

/**
 * Finishes initialition (stows hood?).
**/
void Robot::TeleopInit() {

}
                                    
/**
 * Calls remaining periodic funtions. Checks if robot is shooting, climbing or doing the control panel and calls functions accordingly.
**/
void Robot::TeleopPeriodic() {
   // if (!controls_.Shoot()) { 
        drive_.SetWantCheesyDrive(controls_.Throttle(), controls_.Wheel(),
                                  controls_.QuickTurn());
  //  }

  //check if different contol buttons have been pressed, act accordingly

}

/**
 * Nothing.
**/
void Robot::TestInit() {}

/**
 * Simulates the climbing portion of periodic action. 
**/
void Robot::TestPeriodic() {
  
}

/**
 * Resets a couple things (most zeroing happens in AutonomousInit()). 
**/
void Robot::DisabledInit() {
    
}

/**
 * Updates auto selector.
**/
void Robot::DisabledPeriodic() { }

}  // namespace c2020
}  // namespace team114

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<team114::c2020::Robot>(); }
#endif
