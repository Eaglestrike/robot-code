#pragma once

#include <frc/TimedRobot.h>

#include "subsystems/drive.h"
#include "util/constructor_macros.h"

namespace team114 {
namespace c2020 {

class Robot : public frc::TimedRobot {
    DISALLOW_COPY_ASSIGN(Robot)
   public:
    Robot();
    void RobotInit() override;

    void AutonomousInit() override;
    void AutonomousPeriodic() override;

    void TeleopInit() override;
    void TeleopPeriodic() override;

    void TestInit() override;
    void TestPeriodic() override;

   private:
    Drive& drive_;
};

}  // namespace c2020
}  // namespace team114
