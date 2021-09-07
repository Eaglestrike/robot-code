#pragma once

#include <frc/Joystick.h>
#include <frc/TimedRobot.h>
#include <units/units.h>

#include <optional>

#include "controls.h"


namespace team114 {
namespace c2020 {

class Robot : public frc::TimedRobot {
   public:
    inline static const auto kPeriod = 10_ms;
    Robot();
    void RobotInit() override;
    void RobotPeriodic() override;

    void AutonomousInit() override;
    void AutonomousPeriodic() override;

    void TeleopInit() override;
    void TeleopPeriodic() override;

    void TestInit() override;
    void TestPeriodic() override;

    void DisabledInit() override;
    void DisabledPeriodic() override;

   private:
    frc::Joystick ljoy_;
    frc::Joystick rjoy_;
    frc::Joystick ojoy_;
    conf::RobotConfig cfg;

    // CachingSolenoid brake_{frc::Solenoid{6}};
};

}  // namespace c2020
}  // namespace team114