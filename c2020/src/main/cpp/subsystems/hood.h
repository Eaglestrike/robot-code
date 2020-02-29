#pragma once

#include "shims/minimal_phoenix.h"

#include "config.h"
#include "subsystem.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class Hood : public Subsystem {
    SUBSYSTEM_PRELUDE(Hood)
   public:
    Hood(const conf::HoodConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    void SetWantPosition(double degrees);
    void SetWantStow();
    bool IsAtPosition();

   private:
    enum class LoopState {
        UNINITALIZED,
        ZEROING,
        RUNNING,
    };
    const conf::HoodConfig cfg_;
    LoopState state_;
    TalonSRX talon_;
    int zeroing_position_;
    int setpoint_ticks_;
};

}  // namespace c2020
}  // namespace team114
