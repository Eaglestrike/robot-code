#include "hood.h"

#include <cmath>

namespace team114 {
namespace c2020 {

Hood::Hood() : Hood{conf::GetConfig().hood} {}

Hood::Hood(const conf::HoodConfig& cfg)
    : cfg_{cfg},
      state_{LoopState::UNINITALIZED},
      talon_{cfg.talon_id},
      setpoint_ticks_{0} {
    TalonSRXConfiguration c;
    c.peakCurrentLimit = cfg.current_limit;
    c.peakCurrentDuration = 30;
    c.continuousCurrentLimit = 0.8 * cfg.current_limit;
    c.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    c.closedloopRamp = 0.0;  // disable ramping, should be running a profile
    c.peakOutputForward = 1.0;
    c.peakOutputReverse = -1.0;
    c.nominalOutputForward = 0.0;
    c.nominalOutputReverse = 0.0;
    c.voltageCompSaturation = 12.0;
    // TODO(josh) tune
    // https://phoenix-documentation.readthedocs.io/en/latest/ch14_MCSensor.html#velocity-measurement-filter
    c.velocityMeasurementPeriod = VelocityMeasPeriod::Period_10Ms;
    c.velocityMeasurementWindow = 4;
    // TODO(Josh) tune
    c.slot0.allowableClosedloopError = 0;
    c.slot0.closedLoopPeakOutput = 1.0;
    c.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    c.slot0.integralZone = 0;
    c.slot0.maxIntegralAccumulator = 0;
    // https://phoenix-documentation.readthedocs.io/en/latest/ch16_ClosedLoop.html#calculating-velocity-feed-forward-gain-kf
    c.slot0.kF = 0.0;
    c.slot0.kP = 0.0;
    c.slot0.kI = 0.0;
    c.slot0.kD = 0.0;
    c.motionCruiseVelocity = 0;
    c.motionAcceleration = 0;
    c.motionCurveStrength = 0;
    c.motionProfileTrajectoryPeriod = 0;
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

    // TODO(josh) set frame periods
}

void Hood::Periodic() {
    switch (state_) {
        case LoopState::UNINITALIZED:
            talon_.Set(ControlMode::PercentOutput, 0.0);
            zeroing_position_ = talon_.GetSelectedSensorPosition();
            state_ = LoopState::ZEROING;
            break;
        case LoopState::ZEROING:
            zeroing_position_ -= cfg_.zeroing_vel;
            talon_.Set(ControlMode::PercentOutput,
                       cfg_.zeroing_kp * (zeroing_position_ -
                                          talon_.GetSelectedSensorPosition()));
            if (talon_.GetSupplyCurrent() >= cfg_.current_limit) {
                talon_.Set(ControlMode::PercentOutput, 0.0);
                talon_.SetSelectedSensorPosition(0.0);
                state_ = LoopState::RUNNING;
            }
            break;
        case LoopState::RUNNING:
            talon_.Set(ControlMode::MotionMagic, setpoint_ticks_);
            break;
    }
}
void Hood::Stop() { talon_.Set(ControlMode::PercentOutput, 0.0); }
void Hood::ZeroSensors() { state_ = LoopState::UNINITALIZED; }
void Hood::OutputTelemetry() {}

void Hood::SetWantPosition(double degrees) {
    setpoint_ticks_ = degrees * cfg_.ticks_per_degree;
}
bool Hood::IsAtPosition() {
    auto err = std::abs(talon_.GetSelectedSensorPosition() - setpoint_ticks_);
    auto max_err = 1.0 * cfg_.ticks_per_degree;
    return state_ == LoopState::RUNNING && err < max_err;
}

}  // namespace c2020
}  // namespace team114
