#pragma once

#include <ctre/Phoenix.h>

#include "config.h"
#include "subsystem.h"

namespace team114 {
namespace c2020 {

class Drive : public Subsystem {
    SUBSYSTEM_PRELUDE(Drive)
   public:
    Drive(const DriveConfig& cfg);
    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

   private:
    TalonFX left_master_, right_master_;
    TalonFX left_slave_, right_slave_;
};

}  // namespace c2020
}  // namespace team114
