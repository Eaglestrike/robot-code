#pragma once

namespace DriveConstants {
    //TalonFX Motor Control 
    const int front_left_speedMotor = 11;
    const int front_right_speedMotor = 13;
    const int back_left_speedMotor = 15;
    const int back_right_speedMotor = 17;

    const int front_left_angleMotor = 12;
    const int front_right_angleMotor = 14;
    const int back_left_angleMotor = 16;
    const int back_right_angleMotor = 18;

    //Mag Encoder DIO
    const int front_left_angleEncoderA = 0; 
    const int front_right_angleEncoderA = 2;
    const int back_left_angleEncoderA = 4;
    const int back_right_angleEncoderA = 6;

    const int front_left_angleEncoderB = 1;
    const int front_right_angleEncoderB = 3;
    const int back_left_angleEncoderB = 5;
    const int back_right_angleEncoderB = 7;

    //Swerve Module Constants
    const double wheel_radius = 0.0508;
}

namespace OIConstants{
    //Controls
    const int left_joystick = 0;
    const int right_joystick = 1;
    const int xbox = 2;
}

namespace RobotConstants{

}
