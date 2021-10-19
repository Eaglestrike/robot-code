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

bool Limelight::target_valid(){
    double tv = network_table->GetNumber("tv", 0.0);
    if(tv == 0.0){
        //The target is not visible
        return false;
    }
    return true;
}

void Limelight::setLEDMode(std::string mode) {
    if (mode == "OFF") network_table->PutNumber("ledMode", 1);
    if (mode == "ON") network_table->PutNumber("ledMode", 3);
}