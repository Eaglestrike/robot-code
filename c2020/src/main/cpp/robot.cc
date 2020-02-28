#include "robot.h"

#include <units/units.h>

namespace team114 {
namespace c2020 {

Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      drive_{Drive::GetInstance()},
      climber_{Climber::GetInstance()},
      hood_{Hood::GetInstance()},
      intake_{Intake::GetInstance()},
      ball_path_{BallPath::GetInstance()},
      control_panel_{ControlPanel::GetInstance()},
      robot_state_{RobotState::GetInstance()},
      ljoy_{0},
      rjoy_{1},
      ojoy_{2},
      auto_selector_{auton::AutoModeSelector::GetInstance()},
      auto_executor_{std::make_unique<auton::EmptyAction>()},
      cfg{conf::GetConfig()} {}

void Robot::RobotInit() {}
void Robot::RobotPeriodic() {
    drive_.Periodic();
    ball_path_.Periodic();
    hood_.Periodic();
    intake_.Periodic();
    climber_.Periodic();
    control_panel_.Periodic();
}

void Robot::AutonomousInit() {
    auto mode = auto_selector_.GetSelectedAction();  // heh
    auto_executor_ = auton::AutoExecutor{std::move(mode)};
}
void Robot::AutonomousPeriodic() { auto_executor_.Periodic(); }

void Robot::TeleopInit() {}
void Robot::TeleopPeriodic() {
    double throttle = ljoy_.GetRawAxis(1);
    double wheel = -rjoy_.GetRawAxis(0);
    bool quick_turn = rjoy_.GetRawButton(1);
    drive_.SetWantCheesyDrive(throttle, wheel, quick_turn);

    bool climb_up = ojoy_.GetRawButton(1);
    bool climb_down = ojoy_.GetRawButton(2);
    if (climb_up == climb_down) {
        climber_.SetWantDirection(Climber::Direction::Neutral);
    } else {
        climber_.SetWantDirection(climb_up ? Climber::Direction::Up
                                           : Climber::Direction::Down);
    }

    bool shot_short = ojoy_.GetRawButtonPressed(3);
    bool shot_med = ojoy_.GetRawButtonPressed(4);
    bool shot_long = ojoy_.GetRawButtonPressed(5);
    if (shot_short) {
        ball_path_.SetWantShot(BallPath::ShotType::Short);
    } else if (shot_med) {
        ball_path_.SetWantShot(BallPath::ShotType::Med);
    } else if (shot_long) {
        ball_path_.SetWantShot(BallPath::ShotType::Long);
    }
    bool shoot = ljoy_.GetRawButton(1);
    bool unjam = ojoy_.GetRawButton(6);
    bool intake = ojoy_.GetRawButton(7);
    if (shoot) {
        ball_path_.SetWantState(BallPath::State::Shoot);
    } else if (unjam) {
        ball_path_.SetWantState(BallPath::State::Unjm);
    } else if (intake) {
        ball_path_.SetWantState(BallPath::State::Intk);
    } else {
        ball_path_.SetWantState(BallPath::State::Idle);
    }

    bool deploy = ojoy_.GetRawButton(8);
    bool scoot_f = ojoy_.GetRawButton(9);
    bool scoot_r = ojoy_.GetRawButton(10);
    bool rot_control = ojoy_.GetRawButtonPressed(11);
    bool pos_control_r = ojoy_.GetRawButtonPressed(12);
    bool pos_control_b = ojoy_.GetRawButtonPressed(13);
    bool pos_control_g = ojoy_.GetRawButtonPressed(14);
    bool pos_control_y = ojoy_.GetRawButtonPressed(15);
    control_panel_.SetDeployed(deploy);
    if (pos_control_r) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Red);
    } else if (pos_control_b) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Blue);
    } else if (pos_control_g) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Green);
    } else if (pos_control_y) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Yellow);
    } else if (rot_control) {
        control_panel_.DoRotationControl();
    } else if (scoot_f) {
        control_panel_.Scoot(ControlPanel::ScootDir::Forward);
    } else if (scoot_r) {
        control_panel_.Scoot(ControlPanel::ScootDir::Reverse);
    } else {
        control_panel_.Scoot(ControlPanel::ScootDir::Neutral);
    }
}

void Robot::TestInit() {}
void Robot::TestPeriodic() {
    bool climb_up = ojoy_.GetRawButton(1);
    bool climb_down = ojoy_.GetRawButton(2);
    if (climb_up == climb_down) {
        climber_.SetZeroingWind(Climber::Direction::Neutral);
    } else {
        climber_.SetZeroingWind(climb_up ? Climber::Direction::Up
                                         : Climber::Direction::Down);
    }
}

void Robot::DisabledInit() {
    auto_executor_.Stop();
    climber_.SetWantDirection(Climber::Direction::Neutral);
}
void Robot::DisabledPeriodic() { auto_selector_.UpdateSelection(); }

}  // namespace c2020
}  // namespace team114

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<team114::c2020::Robot>(); }
#endif
