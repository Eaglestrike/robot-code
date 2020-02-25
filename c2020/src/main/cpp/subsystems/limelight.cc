#include "limelight.h"

#include "robot_state.h"
#include "util/number_util.h"

namespace team114 {
namespace c2020 {

Limelight::Limelight() : Limelight(conf::GetConfig().limelight) {}

Limelight::Limelight(conf::LimelightConfig& cfg) : cfg_{std::move(cfg)} {
    network_table_ =
        nt::NetworkTableInstance::GetDefault().GetTable(cfg_.table_name);
}

void Limelight::Periodic() {
    ReadPeriodicIn();
    WritePeriodicOut();
    RobotState::GetInstance().ObserveVision(frc2::Timer::GetFPGATimestamp(),
                                            GetTarget());
}

void Limelight::ReadPeriodicIn() {
    per_in_.latency =
        network_table_->GetNumber("tl", 0) * 1_ms + kImageCaptureLatency;
    per_in_.led_mode = (int)network_table_->GetNumber("ledMode", 1.0);
    per_in_.pipeline = (int)network_table_->GetNumber("pipeline", 0);
    per_in_.x_off = network_table_->GetNumber("tx", 0.0);
    per_in_.y_off = network_table_->GetNumber("ty", 0.0);
    per_in_.area = network_table_->GetNumber("ta", 0.0);
    // paranoid about double equality...
    sees_target_ = network_table_->GetNumber("tv", 0) > 0.5;
}

void Limelight::WritePeriodicOut() {
    if (per_out_.led_mode != per_in_.led_mode ||
        per_out_.pipeline != per_in_.pipeline) {
        // LOG
        per_out_dirty_ = true;
    }
    if (per_out_dirty_) {
        network_table_->PutNumber("ledMode", per_out_.led_mode);
        network_table_->PutNumber("camMode", per_out_.cam_mode);
        network_table_->PutNumber("pipeline", per_out_.pipeline);
        network_table_->PutNumber("stream", per_out_.stream);
        network_table_->PutNumber("snapshot", per_out_.snapshot);
        per_out_dirty_ = false;
    }
}

void Limelight::SetLedMode(LedMode mode) {
    int mode_int = static_cast<int>(mode);
    if (mode_int != per_out_.led_mode) {
        per_out_.led_mode = mode_int;
        per_out_dirty_ = true;
    }
}

void Limelight::SetPipeline(int pipeline) {
    if (pipeline != per_out_.pipeline) {
        // TODO reset veision here
        // RobotState::GetInstance().ResetVision();
        per_out_.pipeline = pipeline;
        // LOG
        per_out_dirty_ = true;
    }
}

int Limelight::GetPipeline() { return per_out_.pipeline; }

void Limelight::ForceFlushOuts() { per_out_dirty_ = true; }

bool Limelight::SeesTarget() { return sees_target_; }

units::second_t Limelight::GetLatency() { return per_in_.latency; }

std::optional<Limelight::TargetInfo> Limelight::GetTarget() {
    if (SeesTarget()) {
        TargetInfo t;
        t.horizontal = DegToRad(per_in_.x_off);
        t.vertical = DegToRad(per_in_.y_off);
        return t;
    }
    return {};
}

}  // namespace c2020
}  // namespace team114
