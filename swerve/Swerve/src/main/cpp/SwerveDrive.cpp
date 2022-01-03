#include "SwerveDrive.h"
#include "math.h"

SwerveDrive::SwerveDrive(){}

void SwerveDrive::drive(double x1, double y1, double x2){
    double r = sqrt(L*L + W*W);
    //y1 *= -1;

    double a = x1 - x2 * (L/r);
    double b = x1 + x2 * (L/r);
    double c = y1 - x2 * (W/r);
    double d = y1 + x2 * (W/r);

    double backRightSpeed = sqrt((a*a) + (d*d));
    double backLeftSpeed = sqrt((a*a) + (c*c));
    double frontRightSpeed = sqrt((b*b) + (d*d));
    double frontLeftSpeed = sqrt((b*b) + (c*c));

    double backRightAngle = atan2(a, d) * 180/ M_PI;
    //backRightAngle = backRightAngle < 0? 180 + backRightAngle + 180 : backRightAngle;
    double backLeftAngle = atan2(a, c) *180/ M_PI;
    double frontRightAngle = atan2(b, d) *180/ M_PI;
    double frontLeftAngle = atan2(b, c) *180/ M_PI;

    backRight.drive(backRightSpeed, backRightAngle);
    backLeft.drive(-backLeftSpeed, backLeftAngle);
    frontRight.drive(frontRightSpeed, frontRightAngle);
    frontLeft.drive(-frontLeftSpeed, frontLeftAngle);
}

void SwerveDrive::SetPID(){
    backRight.setPID();
    backLeft.setPID();
    frontRight.setPID();
    frontLeft.setPID();
}