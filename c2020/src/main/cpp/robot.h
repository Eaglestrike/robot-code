#pragma once

#include <frc/Joystick.h>
#include <frc/TimedRobot.h>
#include <units/units.h>

// Testing
#include <ctre/Phoenix.h>
#include <frc/Solenoid.h>

#include <optional>

#include "auto/executor.h"
#include "auto/selector.h"
#include "robot_state.h"
#include "subsystems/ball_path.h"
#include "subsystems/climber.h"
#include "subsystems/control_panel.h"
#include "subsystems/drive.h"
#include "subsystems/hood.h"
#include "subsystems/intake.h"
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
    Climber& climber_;
    Hood& hood_;
    Intake& intake_;
    BallPath& ball_path_;
    ControlPanel& control_panel_;
    RobotState& robot_state_;
    frc::Joystick ljoy_;
    frc::Joystick rjoy_;
    frc::Joystick ojoy_;
    auton::AutoModeSelector& auto_selector_;
    auton::AutoExecutor auto_executor_;
    conf::RobotConfig cfg;
};

}  // namespace c2020
}  // namespace team114
