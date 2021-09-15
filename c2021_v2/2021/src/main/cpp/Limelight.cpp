#include "Limelight.h"

Limelight::Limelight() {
    network_table =
        nt::NetworkTableInstance::GetDefault().GetTable(table_name);
}; //Set config vars if we ever get a config file

double Limelight::getXOff() {
    return network_table->GetNumber("tx", 10000.0);
}

double Limelight::getYOff() {
    return network_table->GetNumber("ty", 10000.0);
}
