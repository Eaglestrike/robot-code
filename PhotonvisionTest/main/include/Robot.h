// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <string>

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <photonlib/PhotonCamera.h>
#include <photonlib/PhotonUtils.h>
#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>


class Robot : public frc::TimedRobot {
 public:
  void TeleopInit() override;
  void TeleopPeriodic() override;
  

 private:
  photonlib::PhotonCamera camera{"gloworm"};
};
