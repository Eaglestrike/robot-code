#pragma once

#include <WheelDrive.h>
#include "math.h"

class SwerveDrive{
    public:
        SwerveDrive();
        void Drive (double x1, double y1, double x2, double rot, bool fieldOrient);
        void SetPID();
        void ResetEncoders();

    private:

        //Width and Length of the distance between wheel axle
        double m_W = 0.394;
        double m_L = 0.394;

        double m_temp;

        WheelDrive backRight{18, 17, 6, 7};
        WheelDrive backLeft{16, 15, 4, 5};
        WheelDrive frontRight{14, 13, 2, 3};
        WheelDrive frontLeft{12, 11, 0, 1};

};  