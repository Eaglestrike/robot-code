#pragma once

#include <array>
#include <memory>
#include <optional>

#include "frc/geometry/Pose2d.h"
#include "frc/geometry/Rotation2d.h"
#include "frc/geometry/Translation2d.h"
#include "frc/smartdashboard/SmartDashboard.h"
#include "frc2/Timer.h"

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

#include <units/units.h>

#include "config.h"
#include "subsystem.h"

namespace team114 {
namespace c2020 {

class Limelight : public Subsystem {
   public:
    SUBSYSTEM_PRELUDE(Limelight)
    const units::second_t kImageCaptureLatency = 11_ms;

    struct RawTargetInfo {
        double x = 1.0;
        double y;
        double z;
    };
    struct TargetInfo {
        units::radian_t horizontal;
        units::radian_t vertical;
    };
    const conf::LimelightConfig cfg_;

    enum class LedMode : int {
        PIPELINE = 0,
        OFF = 1,
        BLINK = 2,
        ON = 3,
    };

    Limelight(conf::LimelightConfig& cfg);

    void Periodic() final override;

    void SetLedMode(LedMode mode);

    void SetPipeline(int pipeline);

    int GetPipeline();

    void ForceFlushOuts();

    bool SeesTarget();

    units::second_t GetLatency();

    std::shared_ptr<nt::NetworkTable> GetNetworkTable();

    std::optional<TargetInfo> GetTarget();

   private:
    void ReadPeriodicIn();

    void WritePeriodicOut();

    std::shared_ptr<nt::NetworkTable> network_table_;

    struct PeriodicIn {
        units::second_t latency;
        int led_mode;
        int pipeline;
        double x_off;
        double y_off;
        double area;
    };
    struct PeriodicOut {
        int led_mode = 1;  // 0 - use pipeline mode, 1 - off, 2 - blink, 3 - on
        int cam_mode = 0;  // 0 - vision processing, 1 - driver camera
        int pipeline = 0;  // 0 - 9
        int stream = 2;    // sets stream layout if another webcam is attached
        int snapshot = 0;  // 0 - stop snapshots, 1 - 2 Hz
    };
    PeriodicIn per_in_;
    PeriodicOut per_out_;
    bool per_out_dirty_ = true;
    bool sees_target_ = false;
};

}  // namespace c2020
}  // namespace team114
