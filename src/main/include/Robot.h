#pragma once

#include <string>
#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/Joystick.h>
#include <frc/XboxController.h>
#include <frc/Timer.h>
#include <frc/Compressor.h>
#include "Drive.h"
#include "Shoot.h"
#include "Intake.h"
#include "Channel.h"
#include "Climb.h"

frc::Joystick l_joy{0};
frc::Joystick r_joy{1};
frc::XboxController xbox{2};
frc::Compressor compressor{0};

class Robot : public frc::TimedRobot {
  public:
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
    frc::SendableChooser<std::string> m_chooser;
    const std::string kAutoNameDefault = "Default";
    const std::string kAutoNameCustom = "My Auto";
    std::string m_autoSelected;

    Drive _drivetrain;
    Shoot _shooter;
    Intake _intake;
    Channel _channel;
    Climb _climb;
};