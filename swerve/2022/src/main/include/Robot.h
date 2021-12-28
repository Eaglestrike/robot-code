#pragma once

#include <string>
#include "subsystems/Drivetrain.h"

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/Joystick.h>
#include <AHRS.h>
#include <frc/XboxController.h>
#include <frc/SlewRateLimiter.h>
#include <frc/MathUtil.h>
#include <frc/geometry/Rotation2d.h>
#include <frc/geometry/Translation2d.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cstdio>
#include <thread>

#include <cameraserver/CameraServer.h>
#include "Constants.h"


frc::Joystick l_joy{OIConstants::left_joystick};
frc::Joystick r_joy{OIConstants::right_joystick};
frc::XboxController xbox{OIConstants::xbox};
AHRS * navx;

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

  void Drive(bool fieldRelative);

 private:
  Drivetrain m_swerve;
  frc::SlewRateLimiter<units::scalar> xspeedlimiter{3 / 1_s};
  frc::SlewRateLimiter<units::scalar> yspeedlimiter{3 / 1_s};
  frc::SlewRateLimiter<units::scalar> rotlimiter{3 / 1_s};

  frc::SendableChooser<std::string> m_chooser;
  const std::string kAutoNameDefault = "Default";
  const std::string kAutoNameCustom = "My Auto";
  std::string m_autoSelected;

  frc::Rotation2d rotation;
  frc::Translation2d position;
};
