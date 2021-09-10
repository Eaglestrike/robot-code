// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <string>

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/Joystick.h>
#include <frc/XboxController.h>
#include "ctre/Phoenix.h"
#include "frc/WPILib.h"
#include <iostream>

#include <frc/SpeedControllerGroup.h>
#include <frc/controller/PIDController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>
#include <frc/drive/DifferentialDrive.h>

#include <frc/smartdashboard/SmartDashboard.h>


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

    //Joysticks
    frc::Joystick l_joy{0};
    frc::Joystick r_joy{1};
    frc::XboxController xbox{2};

    //TalonFX
    WPI_TalonFX * left_master = new WPI_TalonFX(0);
    WPI_TalonFX * left_slave = new WPI_TalonFX(1);
    WPI_TalonFX * right_master = new WPI_TalonFX(2);
    WPI_TalonFX * right_slave = new WPI_TalonFX(3);
    //turret rot4
    //shooter motors; 5 & 6

    //Stuff for drive
    frc::SpeedControllerGroup left_side{*left_master, *left_slave};
    frc::SpeedControllerGroup right_side{*right_master, *right_slave};
    frc::DifferentialDrive m_drive{left_side, right_side};
};
