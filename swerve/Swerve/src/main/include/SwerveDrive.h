#pragma once

#include <WheelDrive.h>

class SwerveDrive{
    public:
        SwerveDrive();
        void drive (double x1, double y1, double x2);
        void SetPID();

    private:
        //Width and Length of the distance between wheel axle
        double W = 0.394;
        double L = 0.394;

        WheelDrive backRight{18, 17, 6, 7};
        WheelDrive backLeft{16, 15, 4, 5};
        WheelDrive frontRight{14, 13, 2, 3};
        WheelDrive frontLeft{12, 11, 0, 1};

};  