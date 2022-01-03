#include <WheelDrive.h>

WheelDrive::WheelDrive(int angleMotorPort, int speedMotorPort,
    int encoderPortA, int encoderPortB)
    : angleMotor(angleMotorPort), speedMotor(speedMotorPort),
    encoder(encoderPortA, encoderPortB) {

    pidController.EnableContinuousInput(-180, 180);
}

void WheelDrive::drive(double speed, double angle){
    speedMotor.Set(speed);

    double value = NormalizeEncoderValue();

    double turnOutput = std::clamp(pidController.Calculate(value, angle), -0.5, 0.5);

    // frc::SmartDashboard::PutNumber("nValue", value);
    // frc::SmartDashboard::PutNumber("Angle", angle);
    // frc::SmartDashboard::PutNumber("TurnOutput", turnOutput);
    
    angleMotor.Set(turnOutput);
    
}

double WheelDrive::NormalizeEncoderValue(){
    currEncoderValue = encoder.Get();
    int encoderDelta = -1*(currEncoderValue - prevEncoderValue);
    angle += (double) encoderDelta / 1024 * 360;
    angle = (angle > 360) ? angle-360: angle;
    angle = (angle < 0) ? angle+360 : angle;
    prevEncoderValue = currEncoderValue;

    angle = (angle > 180)? angle - 360: angle;
    return angle;
}

void WheelDrive::setPID(){
    double pGain_a = frc::SmartDashboard::GetNumber("P angle", 0.0);
    frc::SmartDashboard::PutNumber("P angle", pGain_a);
    double iGain_a = frc::SmartDashboard::GetNumber("I angle", 0.0);
    frc::SmartDashboard::PutNumber("I angle", iGain_a);
    double dGain_a = frc::SmartDashboard::GetNumber("D angle", 0.0);
    frc::SmartDashboard::PutNumber("D angle", dGain_a);
    pidController.SetPID(pGain_a, iGain_a, dGain_a);

}