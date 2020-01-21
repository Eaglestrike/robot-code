#include <string>

namespace team114 {
namespace c2020 {

struct DriveConfig {
    int left_master_id;
    int left_slave_id;
    int right_master_id;
    int right_slave_id;
};

struct RobotConfig {
    std::string mac_address;
    DriveConfig drive;
};

RobotConfig& GetConfig();

}  // namespace c2020
}  // namespace team114
