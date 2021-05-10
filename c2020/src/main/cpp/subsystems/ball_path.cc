#include "ball_path.h"

#include "subsystems/drive.h"

#include <iostream>

namespace team114 {
namespace c2020 {
/**
* Calls other constructor
**/
BallPath::BallPath() : BallPath(conf::GetConfig()) {}

TalonFXConfiguration s;

/**
* Makes a ball path config, defines member variables
**/
BallPath::BallPath(const conf::RobotConfig& cfg)
    : shooter_cfg_{cfg.shooter},
      channel_cfg_{cfg.ball_channel},
      shooter_master_{shooter_cfg_.master_id},
      shooter_slave{shooter_cfg_.slave_id},
      kicker_{shooter_cfg_.kicker_id},
      serializer_{channel_cfg_.serializer_id},
      channel_{channel_cfg_.channel_id},
      s0_{channel_cfg_.s0_port},
      s1_{channel_cfg_.s1_port},
      s2_{channel_cfg_.s2_port},
      s3_{channel_cfg_.s3_port},
      state_{State::Idle},
      intake_{Intake::GetInstance()},
      limelight_{Limelight::GetInstance()}, 
      hood_{Hood::GetInstance()} {
 /*   TalonSRXConfiguration s;
    s.peakCurrentLimit = 45;
    s.peakCurrentDuration = 30;
    s.continuousCurrentLimit = 38; */
    s.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    s.primaryPID.selectedFeedbackCoefficient = 1.0;
    s.closedloopRamp = 0.300;
    s.peakOutputForward = 1.0;
    s.peakOutputReverse = -1.0;
    s.nominalOutputForward = 0.0;
    s.nominalOutputReverse = 0.0;
    s.voltageCompSaturation = 12.0;
    s.velocityMeasurementPeriod = shooter_cfg_.meas_period;
    s.velocityMeasurementWindow = shooter_cfg_.meas_filter_width;
    s.slot0.allowableClosedloopError = 0;
    s.slot0.closedLoopPeakOutput = 1.0;
    s.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    s.slot0.integralZone = 0;
    s.slot0.maxIntegralAccumulator = 256;
    s.slot0.kF = shooter_cfg_.kF;
    s.slot0.kP = shooter_cfg_.kP;
    s.slot0.kI = shooter_cfg_.kI;
    s.slot0.kD = shooter_cfg_.kD;
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = shooter_master_.ConfigAllSettings(s, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    shooter_master_.EnableVoltageCompensation(true);
   // shooter_master_.EnableCurrentLimit(true);
    shooter_master_.SelectProfileSlot(0, 0);
    shooter_master_.SetNeutralMode(NeutralMode::Coast);
    shooter_master_.SetInverted(true);
    shooter_master_.SetSensorPhase(true);
    conf::SetFramePeriodsForPidTalonFX(shooter_master_);
    shooter_slave.SetNeutralMode(NeutralMode::Coast);
    shooter_slave.Follow(shooter_master_);
    shooter_slave.SetInverted(InvertType::OpposeMaster);
    conf::SetFramePeriodsForSlaveTalonFX(shooter_slave);

    TalonSRXConfiguration r;
    r.peakCurrentLimit = channel_cfg_.current_limit;
    r.peakCurrentDuration = 30;
    r.continuousCurrentLimit = 0.8 * channel_cfg_.current_limit;
    r.peakOutputForward = 1.0; 
    r.peakOutputReverse = -1.0;
    r.nominalOutputForward = 0.0;
    r.nominalOutputReverse = 0.0;
    r.voltageCompSaturation = 12.0;
    // TODO(josh) logs everywhere

    for (int i = 0; i < 10; i++) {
        auto err = serializer_.ConfigAllSettings(r, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    serializer_.EnableVoltageCompensation(true);
    serializer_.EnableCurrentLimit(true);
    serializer_.SetInverted(true);
    serializer_.SetNeutralMode(NeutralMode::Brake);
    conf::SetFramePeriodsForOpenLoopTalon(serializer_);

    for (int i = 0; i < 10; i++) {
        auto err = channel_.ConfigAllSettings(r, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    channel_.EnableVoltageCompensation(true);
    channel_.EnableCurrentLimit(true);
    channel_.SetNeutralMode(NeutralMode::Brake);
    conf::SetFramePeriodsForOpenLoopTalon(channel_);

    for (int i = 0; i < 10; i++) {
        auto err = kicker_.ConfigAllSettings(r, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    kicker_.EnableVoltageCompensation(true);
    kicker_.EnableCurrentLimit(true);
    kicker_.SetNeutralMode(NeutralMode::Brake);
    conf::SetFramePeriodsForOpenLoopTalon(kicker_);
}

/**
* periodic function for the ball path. Depends on state, can unjam balls, shoot a ball, and position the serializer and intake.
**/

void BallPath::Periodic() {
    READING_SDB_NUMERIC(double, shooter_P)  shooter_P;
    READING_SDB_NUMERIC(double, shooter_I)  shooter_I;
    READING_SDB_NUMERIC(double, shooter_D)  shooter_D;

    s.slot0.kP = shooter_P;
    s.slot0.kI = shooter_I;
    s.slot0.kD = shooter_D;

   //s0_ dafuq
    bool s0 = !s0_.Get();
 //   bool s1 = !s0_.Get();
   // bool s2 = !s2_.Get();
    bool s2 = !s2_.Get();

    std::cout << "s0: " << s0 << std::endl;
    std::cout<< "s2: " << s2 << std::endl;
    std::cout <<" " << std::endl;
   

    if (state_ == State::Idle) {
        SetChannelDirection(Direction::Neutral);
        SetSerializerDirection(Direction::Neutral);
        intake_.SetIntaking(Intake::RollerState::NEUTRAL);
        intake_.SetWantPosition(Intake::Position::STOWED);
        //other stuff can be added to make everything stop
    }

    if (state_ == State::Unjm) {
        intake_.SetWantPosition(Intake::Position::INTAKING);
        SetChannelDirection(Direction::Reverse);
        SetSerializerDirection(Direction::Reverse);
        hood_.SetWantStow();
        shooter_master_.NeutralOutput();
        return;
    }
    if (state_ == State::Shoot) {
        intake_.SetWantPosition(Intake::Position::STOWED);
        // UpdateShotFromVision();
        hood_.SetWantPosition(current_shot_.hood_angle);
        shooter_master_.Set(ControlMode::Velocity, current_shot_.flywheel_sp);
        if (shooter_master_.GetClosedLoopError() < 5000) {
            //check with prints
            cout << "minimal shooter error reached" << endl;
            SetSerializerDirection(Direction::Forward);
            SetChannelDirection(Direction::Forward);
        } //intake and kicker
        return; 
    } 
    hood_.SetWantStow();
    shooter_master_.NeutralOutput();

    // the only remaining state diff is the position of the intake
   // Intake::Position commanded_pos = state_ == State::Intk
     //                                    ? Intake::Position::INTAKING
       //                                  : Intake::Position::STOWED;
    if (state_ == State::Intk) {
        intake_.SetIntaking(Intake::RollerState::INTAKING);
        intake_.SetWantPosition(Intake::Position::INTAKING);
        //Have the serializer run 
        SetSerializerDirection(Direction::Forward);
        if (s2) {
            SetChannelDirection(Direction::Neutral);
            SetSerializerDirection(Direction::Neutral);
            return;
        }
        if (s0) {
            SetChannelDirection(Direction::Forward);
            SetSerializerDirection(Direction::Neutral);
        }
        else {
            SetChannelDirection(Direction::Neutral);
            SetSerializerDirection(Direction::Neutral);
        }
    }
    
}
/**
* does nothing, presumable purpose to stop action and reset things
**/
void BallPath::Stop() {}
/**
* does nothing, presumable purpose to reset sensors
**/
void BallPath::ZeroSensors() {}
/**
* does nothing, presumable purpose to output relevant data
**/
void BallPath::OutputTelemetry() {}
/**
* sets ballpath state to the inputed state
**/
void BallPath::SetWantState(BallPath::State s) { state_ = s; }
/**
* sets the current shot's flywheel speed and hood angle atributes to the appropriate angles using auto SHOOOOOOT
**/
void BallPath::SetShot() {
    std::pair<double, double> temp = auto_shoot_calc(limelight_.GetNetworkTable()); 
    current_shot_.flywheel_sp = temp.second;
    current_shot_.hood_angle = temp.first; 

    current_shot_.flywheel_sp = 35000;
  /*  current_shot_.hood_angle = hood_angle;
     READING_SDB_NUMERIC(double, FlyWheelSpeed) flywheel_speed;
     READING_SDB_NUMERIC(double, HoodAngle) hood_angle; */
}
/**
* does nothing, presumable purpose to use camera info to update shot type
**/
void BallPath::UpdateShotFromVision() {
    // find distance to goal and look up pre-computer shot
}
/**
* returns true if the hood, flywheel, and drive systems are ready to shoot
**/
bool BallPath::ReadyToShoot() {
    bool hood = hood_.IsAtPosition();
    SDB_NUMERIC(double, ShooterError)
    shooter_err = std::abs(shooter_master_.GetSelectedSensorVelocity() -
                           current_shot_.flywheel_sp);
    bool flywheel = shooter_err <
                    current_shot_.flywheel_sp * shooter_cfg_.shootable_err_pct;
    // std::cout << "fly_sp, accept_err, err" << current_shot_.flywheel_sp << "
    // "
    //           << (current_shot_.flywheel_sp * shooter_cfg_.shootable_err_pct)
    //           << " " << shooter_err << std::endl;
    bool drive = Drive::GetInstance().OrientedForShot();
  //  std::cout << "shot rdy status " << hood << flywheel << drive << std::endl;
    return hood && flywheel && drive;
}
/**
* Sets the channel to forward, backward, or neutral
**/
void BallPath::SetChannelDirection(BallPath::Direction dir) {
    switch (dir) {
        case BallPath::Direction::Forward:
            channel_.Set(ControlMode::PercentOutput, channel_cfg_.channel_cmd);
            kicker_.Set(ControlMode::PercentOutput, shooter_cfg_.kicker_cmd);
            break;
        case BallPath::Direction::Reverse:
            channel_.Set(ControlMode::PercentOutput, -channel_cfg_.channel_cmd);
            kicker_.Set(ControlMode::PercentOutput, -shooter_cfg_.kicker_cmd);
            break;
        case BallPath::Direction::Neutral:
            channel_.Set(ControlMode::PercentOutput, 0.0);
            kicker_.Set(ControlMode::PercentOutput, 0.0);
            break;
    }
}

// TODO unify enum variant naming across the codebase
/**
*  sets the serilaizer's direction to forward, backwards or neutral 
**/
void BallPath::SetSerializerDirection(BallPath::Direction dir) {
  //  std::cout << "SetSerializerDirection() called" << std::endl;
    switch (dir) {
        case BallPath::Direction::Forward:
            serializer_.Set(ControlMode::PercentOutput,
                            channel_cfg_.serializer_cmd);
         //   intake_.SetIntaking(Intake::RollerState::INTAKING);
            break;
        case BallPath::Direction::Reverse:
            serializer_.Set(ControlMode::PercentOutput,
                            -channel_cfg_.serializer_cmd);
       //     intake_.SetIntaking(Intake::RollerState::OUTTAKING);
            break;
        case BallPath::Direction::Neutral:
            serializer_.Set(ControlMode::PercentOutput, 0.0);
       //     intake_.SetIntaking(Intake::RollerState::NEUTRAL);
            break;
    }
}

    }  // namespace c2020
}  // namespace team114
