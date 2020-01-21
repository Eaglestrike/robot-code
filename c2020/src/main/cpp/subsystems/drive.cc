#include "drive.h"

namespace team114 {
namespace c2020 {

Drive::Drive() : Drive{GetConfig().drive} {}

Drive::Drive(const DriveConfig& cfg)
    : left_master_{cfg.left_master_id},
      right_master_{cfg.right_master_id},
      left_slave_{cfg.left_slave_id},
      right_slave_{cfg.right_slave_id} {}

void Drive::Periodic() {}

void Drive::Stop() {}

void Drive::ZeroSensors() {}

void Drive::OutputTelemetry() {}

}  // namespace c2020
}  // namespace team114
