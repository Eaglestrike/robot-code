controls.cc#include "controls.h"

namespace team114 {
namespace c2020 {

/**
* Constructor, initializes left, right, and other joystick with respective ports.
**/
Controls::Controls() : ljoy_{0}, rjoy_{1}, ojoy_{2} {}
/**
 * returns value for joystick in axis 1 to determine throttle amount.
 **/
double Controls::Throttle() { return -ljoy_.GetRawAxis(1); }
/**
 *   returns value for joystick in axis 0.
**/
double Controls::Wheel() { return rjoy_.GetRawAxis(0); }
/**
 *   Checks if button has been pressed to indicate a quick turn.
**/
bool Controls::QuickTurn() { return rjoy_.GetRawButton(1); }
/**
  *  Checks if joystick has been moved enough to indicate climbing up should start.
**/
bool Controls::ClimbUp() { return ojoy_.GetRawAxis(1) < -kAnalogJoyThreshold; }
/**
 *  Checks if joystick has been moved enough to indicate climbing down should start.
**/
bool Controls::ClimbDown() { return ojoy_.GetRawAxis(1) > kAnalogJoyThreshold; }
/**
 *  check if ball is ready to shoot ball short distance if
**/
bool Controls::ShotShortPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 0 || pov == 315 || pov == 45);
}
/**
*   Check if ball is ready to shoot ball medium distance
**/
bool Controls::ShotMedPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 90 || pov == 135 || pov == 270 ||
                               pov == 225);
}
/**
  *  Check if ready to shoot ball long distance
**/
bool Controls::ShotLongPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 180);
}
/**
 *   Checking if intake should begin.
**/
bool Controls::Intake() { return ojoy_.GetRawAxis(3) > kTriggerThreshold; }
/**
  *  Check if button for unjamming has been pressed.
**/
bool Controls::Unjam() { return ojoy_.GetRawButton(7); }
/**
 *   Checks if button to shoot has been pressed
**/
bool Controls::Shoot() {
    // return ljoy_.GetRawButton(1);
    return ojoy_.GetRawButton(5);
}
/**
 *  Checks if joystick has been moved enough to start deploying panel.
**/
bool Controls::PanelDeploy() { return ojoy_.GetRawAxis(2) > kTriggerThreshold; }
/**
*   Pass on message that scoot left has been pressed
**/
bool Controls::ScootLeftPressed() {
    return scoot_left_.Pressed(scoot_tracker_.PassThroughFeed(
        ojoy_.GetRawAxis(4) < -kAnalogJoyThreshold));
}
/**
  *  Pass on message that  scoot right has been pressed
**/
bool Controls::ScootRightPressed() {
    return scoot_right_.Pressed(scoot_tracker_.PassThroughFeed(
        ojoy_.GetRawAxis(4) > kAnalogJoyThreshold));
}
/**
  *  Pass on message that scoot has been released
**/
bool Controls::ScootReleased() {
    return scoot_released_.Pressed(scoot_tracker_.WasNotHeld());
}
/**
 *   Check if Rot Control button has been pressed
**/
bool Controls::RotControlPressed() { return ojoy_.GetRawButtonPressed(8); }
/**
 *   Check if Pos Control Red button has been pressed
**/
bool Controls::PosControlRedPressed() { return ojoy_.GetRawButtonPressed(2); }
/**
*   Check if Pos Control Yellow button has been pressed
**/
bool Controls::PosControlYellowPressed() {
    return ojoy_.GetRawButtonPressed(4);
}
/**
 *   Check if Pos Control Green button has been pressed
**/
bool Controls::PosControlGreenPressed() { return ojoy_.GetRawButtonPressed(1); }
/**
  *  Check if pos control blue button has been pressed
**/
bool Controls::PosControlBluePressed() { return ojoy_.GetRawButtonPressed(3); }

}  // namespace c2020
}  // namespace team114
