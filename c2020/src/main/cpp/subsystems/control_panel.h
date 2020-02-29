#pragma once

#include "shims/minimal_phoenix.h"

#include "config.h"
#include "subsystem.h"
#include "util/caching_types.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class ControlPanel : public Subsystem {
    SUBSYSTEM_PRELUDE(ControlPanel)
   public:
    ControlPanel(const conf::ControlPanelConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    void SetDeployed(bool deployed);
    void DoRotationControl();
    enum ObservedColor {
        Blue,
        Green,
        Yellow,
        Red,
    };
    void DoPositionControl(ObservedColor current);
    enum ScootDir {
        Forward,
        Reverse,
        Neutral,
    };
    void Scoot(ScootDir dir);

   private:
    void MoveTicks(int ticks);

    const conf::ControlPanelConfig cfg_;
    TalonSRX talon_;
    CachingSolenoid deploy_;
};

}  // namespace c2020
}  // namespace team114
