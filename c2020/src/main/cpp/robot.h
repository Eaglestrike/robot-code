#pragma once

#include <frc/TimedRobot.h>
#include <units/units.h>

#include "subsystems/drive.h"
#include "util/constructor_macros.h"

namespace team114 {
namespace c2020 {

class Robot : public frc::TimedRobot {
    DISALLOW_COPY_ASSIGN(Robot)
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
    Drive& drive_;
};

}  // namespace c2020
}  // namespace team114
