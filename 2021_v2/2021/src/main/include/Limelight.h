#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

class Limelight{
    public:
        Limelight();
        double getXOff();
        double getYOff();
        void setLEDMode(std::string mode);
        std::shared_ptr<nt::NetworkTable> GetNetworkTable();
    private:
        void ReadPeriodicIn();
        std::shared_ptr<nt::NetworkTable> network_table;
        std::string table_name = "limelight";
        double height;
        double angle_above_horizontal;
};