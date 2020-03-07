#include "hood.h"

#include <cmath>
#include "util/number_util.h"

namespace team114 {
namespace c2020 {

Hood::Hood() : Hood{conf::GetConfig().hood} {}

Hood::Hood(const conf::HoodConfig& cfg)
    : cfg_{cfg},
      state_{LoopState::UNINITALIZED},
      talon_{cfg.talon_id},
      setpoint_ticks_{0} {
    TalonSRXConfiguration c;
    c.peakCurrentLimit = cfg_.current_limit;
    c.peakCurrentDuration = 30;
    c.continuousCurrentLimit = 0.8 * cfg_.current_limit;
    c.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    c.closedloopRamp = 0.0;  // disable ramping, should be running a profile
    c.peakOutputForward = 1.0;
    c.peakOutputReverse = -1.0;
    c.nominalOutputForward = 0.0;
    c.nominalOutputReverse = 0.0;
    c.voltageCompSaturation = 12.0;
    c.slot0.allowableClosedloopError = 0;
    c.slot0.closedLoopPeakOutput = 1.0;
    c.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    c.slot0.integralZone = 200;
    c.slot0.maxIntegralAccumulator = 10000;
    c.slot0.kF = 0.0;
    c.slot0.kP = cfg_.kP;
    c.slot0.kI = cfg_.kI;
    c.slot0.kD = cfg_.kD;
    c.motionCruiseVelocity = cfg_.profile_vel;
    c.motionAcceleration = cfg_.profile_acc;
    c.motionCurveStrength = cfg_.ctre_curve_smoothing;
    c.forwardSoftLimitEnable = true;
    c.reverseSoftLimitEnable = true;
    c.forwardSoftLimitThreshold = (cfg_.max_degrees - cfg.min_degrees + 0.25) *
                                  cfg_.ticks_per_degree;  // TODO plus a bit
    c.reverseSoftLimitThreshold =
        -0.25 * cfg_.ticks_per_degree;  // TODO minus a bit
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = talon_.ConfigAllSettings(c, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    talon_.EnableVoltageCompensation(true);
    talon_.EnableCurrentLimit(true);
    talon_.SelectProfileSlot(0, 0);
    talon_.SetNeutralMode(NeutralMode::Brake);
    talon_.SetInverted(true);
    talon_.SetSensorPhase(true);

    conf::SetFramePeriodsForPidTalon(talon_);
}

void Hood::Periodic() {
    switch (state_) {
        case LoopState::UNINITALIZED:
            talon_.Set(ControlMode::PercentOutput, 0.0);
            zeroing_position_ = talon_.GetSelectedSensorPosition();
            talon_.ConfigReverseSoftLimitEnable(false);
            state_ = LoopState::ZEROING;
            break;
        case LoopState::ZEROING:
            zeroing_position_ -= cfg_.zeroing_vel;
            talon_.Set(ControlMode::PercentOutput,
                       cfg_.zeroing_kp * (zeroing_position_ -
                                          talon_.GetSelectedSensorPosition()));
            std::cout << "zero err"
                      << zeroing_position_ - talon_.GetSelectedSensorPosition()
                      << std::endl;
            if (talon_.GetSupplyCurrent() >= cfg_.zeroing_current) {
                talon_.Set(ControlMode::PercentOutput, 0.0);
                talon_.SetSelectedSensorPosition(0.0);
                talon_.ConfigReverseSoftLimitEnable(true);
                state_ = LoopState::RUNNING;
            }
            break;
        case LoopState::RUNNING:
            talon_.Set(ControlMode::MotionMagic, setpoint_ticks_);
            // std::cout << talon_.GetSelectedSensorPosition() << "    "
            //           << setpoint_ticks_ << "    "
            //           << talon_.GetMotorOutputPercent() << "      "
            //           << talon_.GetClosedLoopError() << "      "
            //           << talon_.GetClosedLoopTarget() << std::endl;
            break;
    }
}
void Hood::Stop() { talon_.Set(ControlMode::PercentOutput, 0.0); }
void Hood::ZeroSensors() { state_ = LoopState::UNINITALIZED; }
void Hood::OutputTelemetry() {}

void Hood::SetWantPosition(double degrees) {
    degrees = Clamp(degrees, cfg_.min_degrees, cfg_.max_degrees);
    degrees = cfg_.max_degrees - degrees;
    setpoint_ticks_ = degrees * cfg_.ticks_per_degree;
}

void Hood::SetWantStow() { SetWantPosition(cfg_.max_degrees - 1); }

bool Hood::IsAtPosition() {
    auto err = std::abs(talon_.GetSelectedSensorPosition() - setpoint_ticks_);
    auto max_err = 1.0 * cfg_.ticks_per_degree;
    return (state_ == LoopState::RUNNING) && err < max_err;
}

}  // namespace c2020
}  // namespace team114
