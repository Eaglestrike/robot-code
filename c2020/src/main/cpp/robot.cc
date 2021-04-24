#include "robot.h"

#include <units/units.h>

#include <iostream>

namespace team114 {
namespace c2020 {

/**
 * Instentiates structs for future use.
**/
Robot::Robot()
    : frc::TimedRobot{Robot::kPeriod},
      controls_{},
      drive_{Drive::GetInstance()},
      climber_{Climber::GetInstance()},
      hood_{Hood::GetInstance()},
      intake_{Intake::GetInstance()},
      ball_path_{BallPath::GetInstance()},
      control_panel_{ControlPanel::GetInstance()},
      limelight_{Limelight::GetInstance()},
      robot_state_{RobotState::GetInstance()},
      ljoy_{0},
      rjoy_{1},
      ojoy_{2},
      auto_selector_{auton::AutoModeSelector::GetInstance()},
      auto_executor_{std::make_unique<auton::EmptyAction>()},
      cfg{conf::GetConfig()} {}

/**
 * Nothing.
**/
void Robot::RobotInit() {}


/**
 * Calls period function of select classes. Possibly unfinished?
**/
void Robot::RobotPeriodic() {

    // c.shooter.slave_id = 26;
  /*  READING_SDB_NUMERIC(double, slave_shooter) slave_shooter;
    can::TalonFX* sst = new TalonFX(26); 
    sst->Set(ControlMode::Velocity, slave_shooter);
    std::cout << "slave set velocity: " << slave_shooter << std::endl;
    std::cout<< "slave velocity: " << sst->GetSelectedSensorVelocity() << std::endl;

     // c.shooter.master_id = 25;
    READING_SDB_NUMERIC(double, master_shooter)  master_shooter;
    can::TalonFX* mst = new TalonFX(25); 
    mst->Set(ControlMode::Velocity, master_shooter);
    std::cout << "master set velocity: " << master_shooter << std::endl;
    std::cout<< "master velocity: " << mst->GetSelectedSensorVelocity() << std::endl;
    std::cout << "" << std::endl;*/


    //*****TEMPORARY FOR TESTING MOTORS**********************************************/
  /*  // c.ctrl_panel.talon_id = 31;
    READING_SDB_NUMERIC(double, control_panel)  control_panel;
    can::TalonSRX* cpt = new TalonSRX(31);   
    cpt->Set(ControlMode::PercentOutput, control_panel); */

     // c.intake.rot_talon_id = 41;
  //  READING_SDB_NUMERIC(double, intake_rot) intake_rot;
  //  can::TalonSRX* irt = new TalonSRX(41); 
  //  irt->Set(ControlMode::PercentOutput, 30);
   // std::cout << intake_rot << std::endl;

    //c.intake.roller_talon_id = 42;
   // READING_SDB_NUMERIC(double, roller)  roller;
   // can::TalonSRX* rt = new TalonSRX(42);
  //  rt->Set(ControlMode::PercentOutput, 30); 

  /*   //c.hood.talon_id = 52;
    READING_SDB_NUMERIC(double, hood)  hood;
    can::TalonSRX* ht = new TalonSRX(52); 
    ht->Set(ControlMode::PercentOutput, hood);

    // c.shooter.master_id = 53;
    READING_SDB_NUMERIC(double, master_shooter)  master_shooter;
    can::TalonSRX* mst = new TalonSRX(53); 
    mst->Set(ControlMode::PercentOutput, master_shooter);

    // c.shooter.slave_id = 54;
    READING_SDB_NUMERIC(double, slave_shooter) slave_shooter;
    can::TalonSRX* sst = new TalonSRX(54); 
    sst->Set(ControlMode::PercentOutput, slave_shooter);

     //c.shooter.kicker_id = 51;
    READING_SDB_NUMERIC(double, kicker)  kicker;
    can::TalonSRX* kt = new TalonSRX(51); 
    kt->Set(ControlMode::PercentOutput, kicker);

    // c.ball_channel.serializer_id = 43;
    READING_SDB_NUMERIC(double, serializer)  serializer;
    can::TalonSRX* st = new TalonSRX(43); 
    st->Set(ControlMode::PercentOutput, serializer);

    //c.ball_channel.channel_id = 44;
    READING_SDB_NUMERIC(double, channel)  channel;
    can::TalonSRX* ct = new TalonSRX(44); 
    ct->Set(ControlMode::PercentOutput, channel);

     //c.climber.master_id = 19;
    READING_SDB_NUMERIC(double, master_climber) master_climber;
    can::TalonSRX* cmt = new TalonSRX(19); 
    cmt->Set(ControlMode::PercentOutput, master_climber);

    // c.climber.slave_id = 20;
    READING_SDB_NUMERIC(double, slave_climber)  slave_climber;
    can::TalonSRX* cst = new TalonSRX(20); 
    cst->Set(ControlMode::PercentOutput, slave_climber);


    //END OF MOTOR TESTs
    ///////////
    */


    drive_.Periodic();
    ball_path_.Periodic();
    climber_.Periodic();
    control_panel_.Periodic();
    limelight_.Periodic();

    limelight_.SetLedMode(Limelight::LedMode::OFF);
    // auto dist = robot_state_.GetLatestDistanceToOuterPort();
    // auto ang = robot_state_.GetLatestAngleToOuterPort();

    // if (dist.has_value() && ang.has_value()) {
    //     std::cout << "dist, ang " << dist->second << " " << ang->second
    //               << std::endl;
    // } else {
    //     std::cout << "no target" << std::endl;
    // }
}

/**
 * Zeroing sensors, selecting auto mode.
**/
void Robot::AutonomousInit() {
    drive_.ZeroSensors();
    auto mode = auto_selector_.GetSelectedAction();  // heh
    auto_executor_ = auton::AutoExecutor{std::move(mode)};
    hood_.SetWantPosition(40);
}

/**
 * Calls periodic function of select structs.
**/
void Robot::AutonomousPeriodic() {
    auto_executor_.Periodic();
    hood_.Periodic();
    intake_.Periodic();
}

/**
 * Finishes initialition (stows hood?).
**/
void Robot::TeleopInit() { hood_.SetWantStow(); }

/**
 * Calls remaining periodic funtions. Checks if robot is shooting, climbing or doing the control panel and calls functions accordingly.
**/
void Robot::TeleopPeriodic() {
    hood_.Periodic();
    intake_.Periodic();

    if (!controls_.Shoot()) {
        drive_.SetWantCheesyDrive(controls_.Throttle(), controls_.Wheel(),
                                  controls_.QuickTurn());
    }

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

    if (controls_.Shoot()) {
        ball_path_.SetWantShot(BallPath::ShotType::Long);
        ball_path_.SetWantState(BallPath::State::Shoot);
        drive_.SetWantOrientForShot();
    } else if (controls_.Unjam()) {
        ball_path_.SetWantState(BallPath::State::Unjm);
    } else if (controls_.Intake()) {
        ball_path_.SetWantState(BallPath::State::Intk);
    } else {
        ball_path_.SetWantState(BallPath::State::Idle);
    }

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

/**
 * Nothing.
**/
void Robot::TestInit() {}

/**
 * Simulates the climbing portion of periodic action. 
**/
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

/**
 * Resets a couple things (most zeroing happens in AutonomousInit()). 
**/
void Robot::DisabledInit() {
    auto_executor_.Stop();
    drive_.SetWantRawOpenLoop({0.0_mps, 0.0_mps});
    climber_.SetWantDirection(Climber::Direction::Neutral);
}

/**
 * Updates auto selector.
**/
void Robot::DisabledPeriodic() { auto_selector_.UpdateSelection(); }

}  // namespace c2020
}  // namespace team114

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<team114::c2020::Robot>(); }
#endif
