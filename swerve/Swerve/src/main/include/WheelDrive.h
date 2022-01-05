#pragma once

#include <ctre/Phoenix.h>
#include <frc/Encoder.h>
#include <frc/PIDController.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/controller/PIDController.h>

class WheelDrive{
    public:
        WheelDrive(int angleMotorPort, int speedMotorPort, 
            int encoderPortA, int encoderPortB);
        
        void drive(double speed, double angle);
        void setPID();
        double NormalizeEncoderValue();
    
    private:
        static constexpr double MAX_VOLTS = 1.0;
        int prevEncoderValue = 0;
        int currEncoderValue = 0;
        double angle = 0;

        WPI_TalonFX angleMotor;
        WPI_TalonFX speedMotor;
        frc::Encoder encoder;

        //frc::PIDController pidController{0.0,0,0, encoder, angleMotor};
        frc2::PIDController pidController{0.008,0,0.0001};
};