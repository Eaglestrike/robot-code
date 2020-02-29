#include "controls.h"

namespace team114 {
namespace c2020 {

Controls::Controls() : ljoy_{0}, rjoy_{1}, ojoy_{2} {}

double Controls::Throttle() { return -ljoy_.GetRawAxis(1); }
double Controls::Wheel() { return rjoy_.GetRawAxis(0); }
bool Controls::QuickTurn() { return rjoy_.GetRawButton(1); }
bool Controls::ClimbUp() { return ojoy_.GetRawAxis(1) < -kAnalogJoyThreshold; }
bool Controls::ClimbDown() { return ojoy_.GetRawAxis(1) > kAnalogJoyThreshold; }
bool Controls::ShotShortPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 0 || pov == 315 || pov == 45);
}
bool Controls::ShotMedPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 90 || pov == 135 || pov == 270 ||
                               pov == 225);
}
bool Controls::ShotLongPressed() {
    int pov = ojoy_.GetPOV();
    return shot_short_.Pressed(pov == 180);
}
bool Controls::Intake() { return ojoy_.GetRawAxis(3) > kTriggerThreshold; }
bool Controls::Unjam() { return ojoy_.GetRawButton(7); }
bool Controls::Shoot() { return ljoy_.GetRawButton(1); }
bool Controls::PanelDeploy() { return ojoy_.GetRawAxis(2) > kTriggerThreshold; }
bool Controls::ScootLeftPressed() {
    return scoot_left_.Pressed(scoot_tracker_.PassThroughFeed(
        ojoy_.GetRawAxis(4) < -kAnalogJoyThreshold));
}
bool Controls::ScootRightPressed() {
    return scoot_right_.Pressed(scoot_tracker_.PassThroughFeed(
        ojoy_.GetRawAxis(4) > kAnalogJoyThreshold));
}
bool Controls::ScootReleased() {
    return scoot_released_.Pressed(scoot_tracker_.WasNotHeld());
}
bool Controls::RotControlPressed() { return ojoy_.GetRawButtonPressed(8); }
bool Controls::PosControlRedPressed() { return ojoy_.GetRawButtonPressed(2); }
bool Controls::PosControlYellowPressed() {
    return ojoy_.GetRawButtonPressed(4);
}
bool Controls::PosControlGreenPressed() { return ojoy_.GetRawButtonPressed(1); }
bool Controls::PosControlBluePressed() { return ojoy_.GetRawButtonPressed(3); }

}  // namespace c2020
}  // namespace team114
