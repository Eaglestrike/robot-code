#include "config.h"

#include <fstream>
#include <optional>
#include <string>

namespace team114 {
namespace c2020 {

// WPILib chose GCC 7, so no C++20 designated initializers here

// sim / testing before we have a real robot
const RobotConfig MakeDefaultRobotConfig() {
    RobotConfig c;
    c.mac_address = "aa:bb:cc:dd:ee:ff";
    c.drive.left_master_id = 0;
    c.drive.left_slave_id = 1;
    c.drive.right_master_id = 2;
    c.drive.right_slave_id = 3;
    return c;
}

static std::optional<RobotConfig> CONFIG;

RobotConfig& GetConfig() {
    if (CONFIG.has_value()) {
        return CONFIG.value();
    }
    // TODO(josh): log the chosen config
    std::ifstream ifs("/sys/class/net/eth0/address");
    if (!ifs.fail() or !ifs.is_open()) {
        CONFIG = MakeDefaultRobotConfig();
        return GetConfig();
    }
    std::string rio_mac((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    // TODO(josh) fill in with real RIOs
    // TODO(josh) check for newline in file read
    RobotConfig config_a = MakeDefaultRobotConfig();
    RobotConfig config_b = MakeDefaultRobotConfig();
    if (config_a.mac_address == rio_mac) {
        CONFIG = std::move(config_a);
    } else if (config_a.mac_address == rio_mac) {
        CONFIG = std::move(config_b);
    } else {
        CONFIG = MakeDefaultRobotConfig();
    }
    return GetConfig();
}

}  // namespace c2020
}  // namespace team114
