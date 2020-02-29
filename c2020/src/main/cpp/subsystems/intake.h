#pragma once

#include "shims/minimal_phoenix.h"

#include "config.h"
#include "subsystem.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class Intake : public Subsystem {
    SUBSYSTEM_PRELUDE(Intake)
   public:
    Intake(const conf::IntakeConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    enum class Position {
        STARTING,
        STOWED,
        INTAKING,
    };
    enum class RollerState {
        NEUTRAL,
        INTAKING,
        OUTTAKING,
    };
    void SetWantPosition(Position position);
    bool IsAtPosition();
    void SetIntaking(RollerState state);

   private:
    enum class LoopState {
        UNINITALIZED,
        ZEROING,
        RUNNING,
    };
    const conf::IntakeConfig cfg_;
    LoopState state_;
    TalonSRX rot_talon_;
    TalonSRX roller_talon_;
    int setpoint_ticks_;
    int zeroing_position_;
};

}  // namespace c2020
}  // namespace team114
