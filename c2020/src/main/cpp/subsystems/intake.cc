#include "intake.h"

#include <cmath>
#include "util/number_util.h"

namespace team114 {
namespace c2020 {

Intake::Intake() : Intake{conf::GetConfig().intake} {}

Intake::Intake(const conf::IntakeConfig& cfg)
    : cfg_{cfg},
      state_{LoopState::UNINITALIZED},
      rot_talon_{cfg_.rot_talon_id},
      roller_talon_{cfg_.rot_talon_id},
      setpoint_ticks_{0} {
    TalonSRXConfiguration rot;
    rot.peakCurrentLimit = cfg_.rot_current_limit;
    rot.peakCurrentDuration = 30;
    rot.continuousCurrentLimit = 0.8 * cfg_.rot_current_limit;
    rot.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    rot.primaryPID.selectedFeedbackCoefficient = 1.0;
    rot.closedloopRamp = 0.0;
    rot.peakOutputForward = 1.0;
    rot.peakOutputReverse = -1.0;
    rot.nominalOutputForward = 0.0;
    rot.nominalOutputReverse = 0.0;
    rot.voltageCompSaturation = 12.0;
    rot.slot0.allowableClosedloopError = 0;
    rot.slot0.closedLoopPeakOutput = 1.0;
    rot.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    rot.slot0.integralZone = 0;
    rot.slot0.maxIntegralAccumulator = 256;
    rot.slot0.kF = 0.0;
    rot.slot0.kP = cfg_.kP;
    rot.slot0.kI = cfg_.kI;
    rot.slot0.kD = cfg_.kD;
    rot.forwardSoftLimitEnable = false;
    rot.reverseSoftLimitEnable = true;
    rot.forwardSoftLimitThreshold = 0.0;
    rot.reverseSoftLimitThreshold = -10;
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = rot_talon_.ConfigAllSettings(rot, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    rot_talon_.EnableVoltageCompensation(true);
    rot_talon_.EnableCurrentLimit(true);
    rot_talon_.SelectProfileSlot(0, 0);
    rot_talon_.SetNeutralMode(NeutralMode::Brake);
    conf::SetFramePeriodsForPidTalon(rot_talon_);

    TalonSRXConfiguration roller;
    roller.peakCurrentLimit = cfg_.roller_current_limit;
    roller.peakCurrentDuration = 30;
    roller.continuousCurrentLimit = 0.8 * cfg_.roller_current_limit;
    roller.peakOutputForward = 1.0;
    roller.peakOutputReverse = -1.0;
    roller.nominalOutputForward = 0.0;
    roller.nominalOutputReverse = 0.0;
    roller.voltageCompSaturation = 12.0;
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = roller_talon_.ConfigAllSettings(roller, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    roller_talon_.EnableVoltageCompensation(true);
    roller_talon_.EnableCurrentLimit(true);
    roller_talon_.SetNeutralMode(NeutralMode::Brake);
    conf::SetFramePeriodsForOpenLoopTalon(roller_talon_);
}

void Intake::Periodic() {
    switch (state_) {
        case LoopState::UNINITALIZED:
            rot_talon_.Set(ControlMode::PercentOutput, 0.0);
            state_ = LoopState::ZEROING;
            break;
        case LoopState::ZEROING: {
            // seed relative with absolute
            int abs_pos =
                rot_talon_.GetSensorCollection().GetPulseWidthPosition();
            abs_pos -= cfg_.abs_enc_tick_offset;
            // deal with discontinuities; we know zero is our hard stop
            while (abs_pos < 0) {
                abs_pos += cfg_.abs_ticks_per_rot;
            }
            rot_talon_.SetSelectedSensorPosition(abs_pos *
                                                 cfg_.rel_ticks_per_abs_tick);
            state_ = LoopState::RUNNING;
            break;
        }
        case LoopState::RUNNING: {
            double pos = rot_talon_.GetSelectedSensorPosition();
            double rads_from_vertical =
                cfg_.zeroed_rad_from_vertical + pos * cfg_.rads_per_rel_tick;
            double ff = cfg_.SinekF * std::sin(rads_from_vertical);
            rot_talon_.Set(ControlMode::Position, setpoint_ticks_,
                           DemandType::DemandType_ArbitraryFeedForward, ff);
            break;
        }
    }
}
void Intake::Stop() {
    rot_talon_.Set(ControlMode::PercentOutput, 0.0);
    roller_talon_.Set(ControlMode::PercentOutput, 0.0);
}
void Intake::ZeroSensors() { state_ = LoopState::UNINITALIZED; }
void Intake::OutputTelemetry() {}

void Intake::SetWantPosition(Intake::Position pos) {
    switch (pos) {
        case Intake::Position::STARTING:
            setpoint_ticks_ = 0;
            break;
        case Intake::Position::STOWED:
            setpoint_ticks_ = cfg_.stowed_pos_ticks;
            break;
        case Intake::Position::INTAKING:
            setpoint_ticks_ = cfg_.intaking_pos_ticks;
            break;
    }
}

bool Intake::IsAtPosition() {
    auto err =
        std::abs(rot_talon_.GetSelectedSensorPosition() - setpoint_ticks_);
    auto max_err = 2.0 * M_PI / 180.0 / cfg_.rads_per_rel_tick;
    return state_ == LoopState::RUNNING && err < max_err;
}

void Intake::SetIntaking(Intake::RollerState state) {
    switch (state) {
        case Intake::RollerState::NEUTRAL:
            roller_talon_.Set(ControlMode::PercentOutput, 0.0);
            break;
        case Intake::RollerState::INTAKING:
            roller_talon_.Set(ControlMode::PercentOutput, cfg_.intake_cmd);
            break;
        case Intake::RollerState::OUTTAKING:
            roller_talon_.Set(ControlMode::PercentOutput, -cfg_.intake_cmd);
            break;
    }
}

}  // namespace c2020
}  // namespace team114
