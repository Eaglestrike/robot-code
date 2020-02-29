#pragma once

#include <frc/Joystick.h>

namespace team114 {
namespace c2020 {

class EdgeDetector {
   public:
    bool Pressed(bool observed) {
        bool to_ret = observed && !last_;
        last_ = observed;
        return to_ret;
    }

   private:
    bool last_ = false;
};

class ScootStateTracker {
   public:
    bool PassThroughFeed(bool val) {
        if (val) {
            been_fed_ = true;
        }
        return val;
    }
    bool WasNotHeld() {
        bool to_ret = !been_fed_;
        been_fed_ = false;
        return to_ret;
    }

   private:
    bool been_fed_ = true;
};

class Controls {
   public:
    Controls();

    static constexpr double kTriggerThreshold = 0.3;
    static constexpr double kAnalogJoyThreshold = 0.3;

    double Throttle();
    double Wheel();
    bool QuickTurn();

    bool ClimbUp();
    bool ClimbDown();

    bool ShotShortPressed();
    bool ShotMedPressed();
    bool ShotLongPressed();

    bool Intake();
    bool Unjam();
    bool Shoot();

    bool PanelDeploy();
    bool ScootLeftPressed();
    bool ScootRightPressed();
    bool ScootReleased();
    bool RotControlPressed();
    bool PosControlRedPressed();
    bool PosControlYellowPressed();
    bool PosControlGreenPressed();
    bool PosControlBluePressed();

   private:
    frc::Joystick ljoy_;
    frc::Joystick rjoy_;
    frc::Joystick ojoy_;

    EdgeDetector shot_short_;
    EdgeDetector shot_med_;
    EdgeDetector shot_long_;

    EdgeDetector scoot_left_;
    EdgeDetector scoot_right_;
    ScootStateTracker scoot_tracker_;
    EdgeDetector scoot_released_;
};

}  // namespace c2020
}  // namespace team114
