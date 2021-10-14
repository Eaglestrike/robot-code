#pragma once

#include <string>

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/Joystick.h>

#include "Controls.h"
#include "Drive.h"
#include "Intake.h"
#include "Channel.h"
#include "Shoot.h"

class Robot : public frc::TimedRobot {
 public:

  Robot();

  void RobotInit() override;
  void RobotPeriodic() override;

  void AutonomousInit() override;
  void AutonomousPeriodic() override;

  void TeleopInit() override;
  void TeleopPeriodic() override;

  void DisabledInit() override;
  void DisabledPeriodic() override;

  void TestInit() override;
  void TestPeriodic() override;

 private:
  /*frc::SendableChooser<std::string> m_chooser;
  const std::string kAutoNameDefault = "Default";
  const std::string kAutoNameCustom = "My Auto";
  std::string m_autoSelected;*/

  frc::Timer timer;
  Controls controls;
  Drive drive;
  Intake intake;
  Channel channel;
  Shoot shoot;
};
