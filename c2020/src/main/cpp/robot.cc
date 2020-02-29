#include "robot.h"

#include <units/units.h>

#include <iostream>

namespace team114 {
namespace c2020 {

Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      controls_{},
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
    // ball_path_.Periodic();
    climber_.Periodic();
    control_panel_.Periodic();
}

void Robot::AutonomousInit() {
    auto mode = auto_selector_.GetSelectedAction();  // heh
    auto_executor_ = auton::AutoExecutor{std::move(mode)};
}
void Robot::AutonomousPeriodic() {
    auto_executor_.Periodic();
    // hood_.Periodic();
    // intake_.Periodic();
}

void Robot::TeleopInit() {
    // intake_.SetWantPosition(Intake::Position::INTAKING);
}
void Robot::TeleopPeriodic() {
    // hood_.Periodic();
    // intake_.Periodic();

    drive_.SetWantCheesyDrive(controls_.Throttle(), controls_.Wheel(),
                              controls_.QuickTurn());

    bool climb_up = controls_.ClimbUp();
    bool climb_down = controls_.ClimbDown();
    if (climb_up == climb_down) {
        climber_.SetWantDirection(Climber::Direction::Neutral);
    } else {
        climber_.SetWantDirection(climb_up ? Climber::Direction::Up
                                           : Climber::Direction::Down);
    }

    // since these are manually edge-detected we need to invoke them
    // dont try to make the code cleaner by moving them inline to conditionals
    // bool shot_short = controls_.ShotShortPressed();
    // bool shot_med = controls_.ShotMedPressed();
    // bool shot_long = controls_.ShotLongPressed();
    // if (shot_short) {
    //     ball_path_.SetWantShot(BallPath::ShotType::Short);
    // } else if (shot_med) {
    //     ball_path_.SetWantShot(BallPath::ShotType::Med);
    // } else if (shot_long) {
    //     ball_path_.SetWantShot(BallPath::ShotType::Long);
    // }

    // if (controls_.Shoot()) {
    //     ball_path_.SetWantState(BallPath::State::Shoot);
    // } else if (controls_.Unjam()) {
    //     ball_path_.SetWantState(BallPath::State::Unjm);
    // } else if (controls_.Intake()) {
    //     ball_path_.SetWantState(BallPath::State::Intk);
    // } else {
    //     ball_path_.SetWantState(BallPath::State::Idle);
    // }

    control_panel_.SetDeployed(controls_.PanelDeploy());
    if (controls_.PosControlRedPressed()) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Red);
    } else if (controls_.PosControlBluePressed()) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Blue);
    } else if (controls_.PosControlGreenPressed()) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Green);
    } else if (controls_.PosControlYellowPressed()) {
        control_panel_.DoPositionControl(ControlPanel::ObservedColor::Yellow);
    } else if (controls_.RotControlPressed()) {
        control_panel_.DoRotationControl();
    } else if (controls_.ScootRightPressed()) {
        control_panel_.Scoot(ControlPanel::ScootDir::Forward);
    } else if (controls_.ScootLeftPressed()) {
        control_panel_.Scoot(ControlPanel::ScootDir::Reverse);
    } else if (controls_.ScootReleased()) {
        control_panel_.Scoot(ControlPanel::ScootDir::Neutral);
    }
}

void Robot::TestInit() {}
void Robot::TestPeriodic() {
    bool climb_up = controls_.ClimbUp();
    bool climb_down = controls_.ClimbDown();
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
