#include "SwerveDrive.h"


SwerveDrive::SwerveDrive(){}

//Robot Oriented Drive Kinematics
void SwerveDrive::robotOrientDrive(double x1, double y1, double x2){
    double r = sqrt(L*L + W*W);
    y1 *= -1;
    x1 *= -1;
    x2 *= -1;

    double a = x1 - x2 * (L/r);
    double b = x1 + x2 * (L/r);
    double c = y1 - x2 * (W/r);
    double d = y1 + x2 * (W/r);

    double backRightSpeed = sqrt((a*a) + (d*d));
    double backLeftSpeed = sqrt((a*a) + (c*c));
    double frontRightSpeed = sqrt((b*b) + (d*d));
    double frontLeftSpeed = sqrt((b*b) + (c*c));

    double backRightAngle = atan2(a, d) * 180/ M_PI;
    double backLeftAngle = atan2(a, c) *180/ M_PI;
    double frontRightAngle = atan2(b, d) *180/ M_PI;
    double frontLeftAngle = atan2(b, c) *180/ M_PI;

    backRight.drive(backRightSpeed, backRightAngle);
    backLeft.drive(-backLeftSpeed, backLeftAngle);
    frontRight.drive(frontRightSpeed, frontRightAngle);
    frontLeft.drive(-frontLeftSpeed, frontLeftAngle);
}

void SwerveDrive::fieldOrientDrive(double x1, double y1, double x2, double rot){
    double r = sqrt(L*L + W*W);
    y1 *= -1;
    x1 *= -1;
    x2 *= -1;
    rot *= M_PI/180;
    temp = y1*cos(rot) + x1*sin(rot);
    x1 = -y1*sin(rot) + x1*cos(rot);
    y1 = temp;

    double a = x1 - x2 * (L/r);
    double b = x1 + x2 * (L/r);
    double c = y1 - x2 * (W/r);
    double d = y1 + x2 * (W/r);

    double backRightSpeed = sqrt((a*a) + (d*d));
    double backLeftSpeed = sqrt((a*a) + (c*c));
    double frontRightSpeed = sqrt((b*b) + (d*d));
    double frontLeftSpeed = sqrt((b*b) + (c*c));

    double backRightAngle = atan2(a, d) * 180/ M_PI;
    double backLeftAngle = atan2(a, c) *180/ M_PI;
    double frontRightAngle = atan2(b, d) *180/ M_PI;
    double frontLeftAngle = atan2(b, c) *180/ M_PI;

    frc::SmartDashboard::PutNumber("Rotation",rot);

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