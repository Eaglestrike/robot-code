#include "SwerveDrive.h"


SwerveDrive::SwerveDrive(){}

void SwerveDrive::Drive(double x1, double y1, double x2, double rot, bool fieldOrient){
    double r = sqrt(m_L*m_L + m_W*m_W);
    y1 *= -1;
    x1 *= -1;
    rot *= M_PI/180;

    if(fieldOrient){
        m_temp = y1*cos(rot) + x1*sin(rot);
        x1 = -y1*sin(rot) + x1*cos(rot);
        y1 = m_temp;
    }

    double a = x1 - x2 * (m_L/r);
    double b = x1 + x2 * (m_L/r);
    double c = y1 - x2 * (m_W/r);
    double d = y1 + x2 * (m_W/r);

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

void SwerveDrive::ResetEncoders(){
    backRight.ResetEncoder();
    backLeft.ResetEncoder();
    frontRight.ResetEncoder();
    frontLeft.ResetEncoder();
}