#pragma once

#include <iostream>
#include <ctre/Phoenix.h>
#include "frc/WPILib.h"
#include <frc/XboxController.h>

class Shoot{
    public:
        Shoot();
        void Periodic(const frc::XboxController & xbox);
        void Aim();
        void Auto();

    private:
        WPI_TalonFX * turr = new WPI_TalonFX(4);
        WPI_TalonFX * shoot_master = new WPI_TalonFX(5);
        WPI_TalonFX * shoot_slave = new WPI_TalonFX(6);

};