#pragma once

#include "shims/minimal_phoenix.h"

#include "config.h"
#include "subsystem.h"
#include "util/caching_types.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class Climber : public Subsystem {
    SUBSYSTEM_PRELUDE(Climber)
   public:
    Climber(const conf::ClimberConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    enum class Direction {
        Neutral,
        Up,
        Down,
    };
    void SetWantDirection(Direction direction);

    // Used to zero the climber pre-match
    void SetZeroingWind(Direction direction);
    void AssumeZeroed();

   private:
    const conf::ClimberConfig cfg_;
    TalonSRX master_talon_;
    TalonSRX slave_talon_;
    CachingSolenoid latch_;
    CachingSolenoid brake_;

    enum class CurrentAction {
        Climbing,
        Resetting,
    };
    CurrentAction action_;
};

}  // namespace c2020
}  // namespace team114
