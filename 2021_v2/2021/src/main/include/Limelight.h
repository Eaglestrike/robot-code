#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

class Limelight{
    public:
        Limelight();
        double getXOff();
        double getYOff();
        void setLEDMode(std::string mode);
        std::shared_ptr<nt::NetworkTable> GetNetworkTable();

        double height;
        double angle_above_horizontal;
    private:
        void ReadPeriodicIn();
        std::shared_ptr<nt::NetworkTable> network_table;
        std::string table_name = "limelight";
};