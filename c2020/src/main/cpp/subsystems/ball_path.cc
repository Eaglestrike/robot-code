#include "ball_path.h"

#include <iostream>

namespace team114 {
namespace c2020 {

BallPath::BallPath() : BallPath(conf::GetConfig()) {}

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
      hood_{Hood::GetInstance()} {
    TalonSRXConfiguration s;
    s.peakCurrentLimit = 45;
    s.peakCurrentDuration = 30;
    s.continuousCurrentLimit = 38;
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
    shooter_master_.EnableCurrentLimit(true);
    shooter_master_.SelectProfileSlot(0, 0);
    shooter_master_.SetNeutralMode(NeutralMode::Coast);
    shooter_master_.SetInverted(true);
    shooter_master_.SetSensorPhase(true);
    conf::SetFramePeriodsForPidTalon(shooter_master_);
    shooter_slave.SetNeutralMode(NeutralMode::Coast);
    shooter_slave.Follow(shooter_master_);
    shooter_slave.SetInverted(InvertType::FollowMaster);
    conf::SetFramePeriodsForSlaveTalon(shooter_slave);

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

void BallPath::Periodic() {
    bool s0 = !s0_.Get();
    bool s1 = !s1_.Get();
    bool s2 = !s2_.Get();
    bool s3 = !s3_.Get();
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
        SetSerializerDirection(Direction::Neutral);
        if (!s3 || ReadyToShoot()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
            SDB_NUMERIC(double, ReadShooterSpeed)
            _shooter_speed = shooter_master_.GetSelectedSensorVelocity();
#pragma GCC diagnostic pop
            SetChannelDirection(Direction::Forward);
            SetSerializerDirection(Direction::Forward);
        } else {
            SetChannelDirection(Direction::Neutral);
            SetSerializerDirection(Direction::Neutral);
        }
        return;
    }
    hood_.SetWantStow();
    shooter_master_.NeutralOutput();

    // the only remaining state diff is the position of the intake
    Intake::Position commanded_pos = state_ == State::Intk
                                         ? Intake::Position::INTAKING
                                         : Intake::Position::STOWED;
    // process ball movement with sensors:
    // std::cout << s1 << s2 << s3 << std::endl;
    if (s3) {
        SetChannelDirection(Direction::Neutral);
        if (state_ == State::Intk && !s0) {
            SetSerializerDirection(Direction::Forward);
        } else {
            SetSerializerDirection(Direction::Neutral);
        }
        if (s0) {
            intake_.SetWantPosition(Intake::Position::STOWED);
        } else {
            intake_.SetWantPosition(commanded_pos);
        }
    } else if (!s1 && !s2) {
        SetSerializerDirection(Direction::Forward);
        SetChannelDirection(Direction::Neutral);
        intake_.SetWantPosition(commanded_pos);
    } else if (s1 && !s2) {
        SetSerializerDirection(Direction::Forward);
        SetChannelDirection(Direction::Forward);
        intake_.SetWantPosition(commanded_pos);
    } else if (s1 && s2) {
        SetSerializerDirection(Direction::Neutral);
        SetChannelDirection(Direction::Forward);
        intake_.SetWantPosition(commanded_pos);
    } else if (!s1 && s2) {
        SetSerializerDirection(Direction::Forward);
        SetChannelDirection(Direction::Forward);
        intake_.SetWantPosition(commanded_pos);
    } else {
        // LOG uh oh
    }
}
void BallPath::Stop() {}
void BallPath::ZeroSensors() {}
void BallPath::OutputTelemetry() {}

void BallPath::SetWantState(BallPath::State s) { state_ = s; }

void BallPath::SetWantShot(BallPath::ShotType shot) {
    READING_SDB_NUMERIC(double, FlyWheelSpeed) flywheel_speed;
    READING_SDB_NUMERIC(double, HoodAngle) hood_angle;
    current_shot_.flywheel_sp = flywheel_speed;
    current_shot_.hood_angle = hood_angle;
    // switch (shot) {
    //     case BallPath::ShotType::Short:
    //         current_shot_.flywheel_sp = 20000;
    //         current_shot_.hood_angle = 40;
    //         break;
    //     case BallPath::ShotType::Med:
    //         current_shot_.flywheel_sp = 32000;
    //         current_shot_.hood_angle = 40;
    //         break;
    //     case BallPath::ShotType::Long:
    //         current_shot_.flywheel_sp = 40000;
    //         current_shot_.hood_angle = 20;
    //         break;
    // }
}

void BallPath::UpdateShotFromVision() {
    // find distance to goal and look up pre-computer shot
}

bool BallPath::ReadyToShoot() {
    bool hood = hood_.IsAtPosition();
    SDB_NUMERIC(double, ShooterError)
    shooter_err = std::abs(shooter_master_.GetSelectedSensorVelocity() -
                           current_shot_.flywheel_sp);
    bool flywheel = shooter_err <
                    current_shot_.flywheel_sp * shooter_cfg_.shootable_err_pct;
    std::cout << "fly_sp, accept_err, err" << current_shot_.flywheel_sp << " "
              << (current_shot_.flywheel_sp * shooter_cfg_.shootable_err_pct)
              << " " << shooter_err << std::endl;
    // when we have vision, check the drivetrain is oriented
    return hood && flywheel;
}

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

void BallPath::SetSerializerDirection(BallPath::Direction dir) {
    switch (dir) {
        case BallPath::Direction::Forward:
            serializer_.Set(ControlMode::PercentOutput,
                            channel_cfg_.serializer_cmd);
            intake_.SetIntaking(Intake::RollerState::INTAKING);
            break;
        case BallPath::Direction::Reverse:
            serializer_.Set(ControlMode::PercentOutput,
                            -channel_cfg_.serializer_cmd);
            intake_.SetIntaking(Intake::RollerState::OUTTAKING);
            break;
        case BallPath::Direction::Neutral:
            serializer_.Set(ControlMode::PercentOutput, 0.0);
            intake_.SetIntaking(Intake::RollerState::NEUTRAL);
            break;
    }
}

}  // namespace c2020
}  // namespace team114
