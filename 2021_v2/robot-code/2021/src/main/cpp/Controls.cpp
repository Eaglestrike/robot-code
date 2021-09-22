#include "Controls.h"

void Controls::Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    
    _drivetrain.Periodic(l_joy, r_joy);
    _shooter.Manual_Turret(xbox);

    //Button 1 is A
    if(xbox.GetRawButton(1)){
        _shooter.setState(Shoot::State::Aiming);
        _shooter.Periodic();
    } else {
        _shooter.setState(Shoot::State::Idle);
    }

    //Button 2 is B?
    if(xbox.GetRawButton(2)){
        if(!_intake.Deployed()){
            _intake.Deploy();
        }
        _intake.Run();
        _channel.Run();
    } else {
        _intake.Retract();
    }
}

void Controls::Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);
    _shooter.Manual(xbox);
}

void Controls::Auto(){
    _drivetrain.Auto();
}

void Controls::Testing(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    //_shooter.Calibration();
}

void Controls::Zero(){
    _shooter.Zero();
}