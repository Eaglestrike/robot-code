#pragma once

#include <frc/TimedRobot.h>

namespace team114 {
namespace c2020 {

class Robot : public frc::TimedRobot {
   public:
    void RobotInit() override;

    void AutonomousInit() override;
    void AutonomousPeriodic() override;

    void TeleopInit() override;
    void TeleopPeriodic() override;

    void TestInit() override;
    void TestPeriodic() override;

   private:
};

}  // namespace c2020
}  // namespace team114
