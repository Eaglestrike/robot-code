#include "drive.h"

#include <thread>

#include <frc/SPI.h>
#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>
#include <frc2/Timer.h>

#include <units/units.h>

#include "number_util.h" 

namespace team114 {
namespace c2020 {

/**
 * The constructor called if no config is passed in, 
 * in which case it calls the second constructor anyways by explicitly getting the drive config and passing it in
**/
Drive::Drive() : Drive{conf::GetConfig().drive} {}

/**
 * The constructor that takes in a driveconfig to initialize all the drive's members,
 * and the values of the members' variables
**/
Drive::Drive(const conf::DriveConfig& cfg)
    : left_master_{cfg.left_master_id},
      right_master_{cfg.right_master_id},
      left_slave_{cfg.left_slave_id},
      right_slave_{cfg.right_slave_id},
      cfg_{cfg},
      robot_state_{RobotState::GetInstance()},
      kinematics_{cfg.track_width},
      odometry_{{}},
      ramsete_{},
      vision_rot_{cfg_.orient_kp,
                  cfg_.orient_ki,
                  cfg_.orient_kd,
                  {2.0_rad / 1_s, 6.0_rad / 1.0_s / 1.0_s},
                  10_ms},
      has_vision_target_{false} {
    conf::DriveFalconCommonConfig(left_master_);
    conf::DriveFalconCommonConfig(right_master_);
    conf::DriveFalconCommonConfig(left_slave_);
    conf::DriveFalconCommonConfig(right_slave_);
    left_master_.SetInverted(false);
    right_master_.SetInverted(true);
    left_slave_.Follow(left_master_);
    right_slave_.Follow(right_master_);
    left_slave_.SetInverted(InvertType::FollowMaster);
    right_slave_.SetInverted(InvertType::FollowMaster);

    conf::SetDriveMasterFramePeriods(left_master_);
    conf::SetDriveMasterFramePeriods(right_master_);
    conf::SetDriveSlaveFramePeriods(left_slave_);
    conf::SetDriveSlaveFramePeriods(right_slave_);
    CheckFalconFramePeriods();

    vision_rot_.SetIntegratorRange(-0.05, 0.05);
    vision_rot_.SetTolerance(0.02_rad, 0.2_rad / 1.0_s);
}

/**
 * The first method called in Periodic(), in which the slaves are checked whether a reset has occured
 * If a slave was reset, its frame period will once again be set to the correct value like in the constructor
 * A counter will also be ticked to show how many times the falcons have been reset 
 * 
 * https://phoenix-documentation.readthedocs.io/en/latest/ch18_CommonAPI.html#can-bus-utilization-error-metrics
**/
void Drive::CheckFalconFramePeriods() {
    if (left_master_.HasResetOccurred()) {
        conf::SetDriveMasterFramePeriods(left_master_);
        falcon_reset_count_++;
    }
    if (right_master_.HasResetOccurred()) {
        conf::SetDriveMasterFramePeriods(right_master_);
        falcon_reset_count_++;
    }
    if (left_slave_.HasResetOccurred()) {
        conf::SetDriveSlaveFramePeriods(left_slave_);
        falcon_reset_count_++;
    }
    if (right_slave_.HasResetOccurred()) {
        conf::SetDriveSlaveFramePeriods(right_slave_);
        falcon_reset_count_++;
    }
}

/**
 * This method is the most important as it is periodically called, and is what allows the robot to move
 * After checking for motor resets, the current state will be updated
 * Depending on the state, different cotnrollers will be updated such that the robot can perform the movement necessary
 * to follow a path or orient the shooter.
 * Outputs will also be written for analysis 
**/
void Drive::Periodic() {
    CheckFalconFramePeriods();
    UpdateRobotState();
    switch (state_) {
        case DriveState::OPEN_LOOP:
            // TODO(josh)
            break;
        case DriveState::FOLLOW_PATH:
            UpdatePathController();
            break;
        case DriveState::SHOOT_ORIENT:
            UpdateOrientController();
        default:
            // TODO(josh) log here
            break;
    }
    WriteOuts();
}

/**
 * Empty method, presumably was supposed to be used to stop the drive
**/
void Drive::Stop() {}

/**
 * Resets the position of sensors on the drive
**/
void Drive::ZeroSensors() {
    robot_state_.ResetFieldToRobot();
    WaitForNavxCalibration(0.5);
  //  navx_.ZeroYaw();
    left_master_.SetSelectedSensorPosition(0);
    right_master_.SetSelectedSensorPosition(0);
    odometry_.ResetPosition({}, GetYaw());
}

/**
 * Gives the NavX (navigation sensor for field oriented, auto balancing, collision detection, etc...) a time interval to calibrate
 * Calibration retries, failures, and successes are supposed to be log, but atm nothing has been written to log it
**/
/* void Drive::WaitForNavxCalibration(double timeout_sec) {
    frc::Timer time;
    time.Start();
    while (navx_.IsCalibrating() && time.Get() < timeout_sec) {
        // LOG calib retry
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (navx_.IsCalibrating()) {
        // LOG failed to calibrate
    } else {
        // LOG success
    }
} */

/**
 * Empty method, presumably to ... output telemetry
**/
void Drive::OutputTelemetry() {}

/**
 * Adds the drive trajectory it wants to move at, and changes the current state to follow that path
**/
void Drive::SetWantDriveTraj(frc::Trajectory&& traj) {
    curr_traj_.emplace(std::move(traj));
    state_ = DriveState::FOLLOW_PATH;
    traj_timer.Reset();
    traj_timer.Start();
}

/**
 * Updates the odometry of the drive, such that the robot's relative position and pose can be predicted
**/
void Drive::UpdateRobotState() {
    odometry_.Update(GetYaw(), GetEncoder(left_master_),
                     GetEncoder(right_master_));
    robot_state_.ObserveFieldToRobot(frc2::Timer::GetFPGATimestamp(),
                                     odometry_.GetPose()); 
}

/**
 * Updates the path controller such that it'll correctly follow the trajectory the driver has set for it
**/
void Drive::UpdatePathController() {
    if (!curr_traj_.has_value()) {
        // LOG
    }
    if (FinishedTraj()) {
        curr_traj_.reset();
        SetWantRawOpenLoop({0.0_mps, 0.0_mps});
    }
    auto time_along = traj_timer.Get();
    auto desired = curr_traj_.value().Sample(units::second_t{time_along});
    auto chassis_v = ramsete_.Calculate(
        robot_state_.GetLatestFieldToRobot().second, desired);
    auto wheel_v = kinematics_.ToWheelSpeeds(chassis_v);
    // convert into falcon 500 internal encoder ticks
    auto MetersPerSecToTicksPerDecisec =
        [this](units::meters_per_second_t meters_per_second) -> double {
        return meters_per_second / cfg_.meters_per_falcon_tick * 1_s / 10.0;
    };
    // TODO(josh) consider using WPILib
    // SimpleMotorFeedForward to add kA term
    pout_.control_mode = ControlMode::Velocity;
    pout_.left_demand = MetersPerSecToTicksPerDecisec(wheel_v.left);
    pout_.right_demand = MetersPerSecToTicksPerDecisec(wheel_v.right);
}

bool Drive::BackUp(double dist) { //units are meters
    //do pid but until desired # of ticks works
    double wheel_diameter = 5.75; //inches
    double circumference = wheel_diameter*M_PI;
    double rotations = dist/circumference;
    double ticks = -rotations*22000; 

    double ticks_gone = left_master_.GetSelectedSensorPosition();
    double Kp = -0.00003;
    double Ki = -0.00002;
    double error = abs(ticks) - abs(ticks_gone);
    double correction = Kp*error + Ki;

    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = correction;
    pout_.right_demand = correction;

    std::cout << "ticks: " << ticks << std::endl;
    std::cout << "ticks gone: " << ticks_gone << std::endl;
    std::cout << "correction: " << correction << std::endl;

    return abs(correction) < 0.1;

}


/**
 * Changes the current state of the robot to orient its vision sensor for a shot, and sets the angle at which the sensor should be rotated to
**/
/*void Drive::SetWantOrientForShot(Limelight& limelight, double Kp, double Ki, double Kd) {
    state_ = DriveState::SHOOT_ORIENT;
    vision_rot_.SetGoal(0.0_rad);

	Kp = 0.017; 
    Ki = 0.015;

	double x_off = limelight.GetNetworkTable()->GetNumber("tx", 0.0);

	//auto position robot so x_off is reasonable
	//https://docs.limelightvision.io/en/latest/cs_aiming.html
	double heading_error = -x_off;
	double steering_adjust = 0.0;
	steering_adjust = Kp*heading_error + Ki;
    if (steering_adjust < -1) steering_adjust = 1;
    if (steering_adjust > 1) steering_adjust = 1;
  //  std::cout <<"x offset: " << x_off << std::endl; 
    std::cout << "steering adjust: " << steering_adjust << std::endl; 
    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = steering_adjust;
    pout_.right_demand = -steering_adjust; 
}*/

/**
 * Returns true if the vision sensor has correctly rotated to the position set
**/
/*bool Drive::OrientedForShot(Limelight& limelight) {
    double x_off = limelight.GetNetworkTable()->GetNumber("tx", 0.0);
    std::cout << x_off << std::endl;
    return (x_off < 2);
}*/

/**
 * Called if the current drive state is to orient for a shot
 * It updates the vision controller, which determines the movement the vision sensor takes towards the goal
**/
void Drive::UpdateOrientController() {
  /*  auto latest = robot_state_.GetLatestAngleToOuterPort();
    if (!latest.has_value()) {
        return;
        has_vision_target_ = false;
    }
    READING_SDB_NUMERIC(double, RotKp) kp;
    READING_SDB_NUMERIC(double, RotKi) ki;
    READING_SDB_NUMERIC(double, RotKd) kd;
    vision_rot_.SetPID(kp, ki, kd);
    READING_SDB_NUMERIC(double, RotVel) vel;
    READING_SDB_NUMERIC(double, RotAcc) acc;
    vision_rot_.SetConstraints({static_cast<double>(vel) * 1_rad / 1_s,
                                static_cast<double>(acc) * 1_rad / 1_s / 1_s});
  //  std::cout << "read gains " << kp << " " << ki << " " << kd << " " << vel << " " << acc << std::endl;
    has_vision_target_ = true;
    auto err = latest.value().second;
    double demand = vision_rot_.Calculate(err);
    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = -demand;
    pout_.right_demand = demand;
  //  std::cout << "orient w/ tgt, dmd " << err << " " << demand << std::endl; */
}

/**
 * Returns true if the current trajectory variable does not have a value, meaning it is not currently moving
**/
bool Drive::FinishedTraj() {
    if (!curr_traj_.has_value()) {
        // not running one at the moment
        return true;
    }
    // TODO(josh) find a better way?
    return traj_timer.Get() > curr_traj_.value().TotalTime();
}

/**
 * Sets the drive's state to an open loop, meaning nothing gets updated everytime Periodic() is called
**/
void Drive::SetWantRawOpenLoop(
    const frc::DifferentialDriveWheelSpeeds& openloop) {
    state_ = DriveState::OPEN_LOOP;
    pout_.control_mode = ControlMode::PercentOutput;
    pout_.left_demand = openloop.left.to<double>();
    pout_.right_demand = openloop.right.to<double>();
}

/**
 * Taken from 254, it changes the controls and feel of manipulating the drive to that of a curvature drive
 * https://www.reddit.com/r/FRC/comments/80679m/what_is_curvature_drive_cheesy_drive/  
**/
void Drive::SetWantCheesyDrive(double throttle, double wheel, bool quick_turn) {
    throttle = Deadband(throttle, 0.05);
    wheel = Deadband(wheel, 0.035);

    constexpr double kWheelGain = 1.97;
    constexpr double kWheelNonlinearity = 0.05;
    const double denominator = std::sin(M_PI / 2.0 * kWheelNonlinearity);
    // Apply a sin function that's scaled to make it feel better.
    if (!quick_turn) {
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = std::sin(M_PI / 2.0 * kWheelNonlinearity * wheel);
        wheel = wheel / (denominator * denominator) * std::abs(throttle);
    }
    wheel *= kWheelGain;
    frc::DifferentialDriveWheelSpeeds signal = kinematics_.ToWheelSpeeds(
        frc::ChassisSpeeds{units::meters_per_second_t{throttle}, 0.0_mps,
                           units::radians_per_second_t{wheel}});
    signal.Normalize(1.0_mps);
    SetWantRawOpenLoop(signal);
}

/**
 * Writes the relevant outputs  
**/
void Drive::WriteOuts() {
   // SDB_NUMERIC(double, LeftDriveTalonDemand){pout_.left_demand};
  //  SDB_NUMERIC(double, RightDriveTalonDemand){pout_.right_demand};
    left_master_.Set(pout_.control_mode, pout_.left_demand);
    right_master_.Set(pout_.control_mode, pout_.right_demand);
 //   if (pout_.left_demand == 0 && pout_.right_demand == 0) return;
  //  std::cout << "write out left demand: " << pout_.left_demand << std::endl;
  //  std::cout << "write out right demand: " << pout_.right_demand << std::endl;
}

/**
 * Gets the rotation of the drive in terms of radians
**/
constexpr auto RAD_PER_DEGREE = units::constants::pi * 1_rad / 180.0;
/*frc::Rotation2d Drive::GetYaw() {
    return frc::Rotation2d(navx_.GetYaw() * RAD_PER_DEGREE);
}*/

/**
 * Simple getter method to get the encoder object of the Talon motor controller
**/
units::meter_t Drive::GetEncoder(TalonFX& master_talon) {
    return (static_cast<double>(master_talon.GetSelectedSensorPosition())) *
           cfg_.meters_per_falcon_tick;
}

}  // namespace c2020
}  // namespace team114
