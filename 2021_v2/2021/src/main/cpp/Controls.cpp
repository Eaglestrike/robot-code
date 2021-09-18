#include "Controls.h"

void Controls::Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    
    _drivetrain.Periodic(l_joy, r_joy);

    if(xbox.GetRawButton(1)){
        _shooter.setState(Shoot::State::Aiming);
        _shooter.Periodic();
    } else {
        _shooter.setState(Shoot::State::Idle);
    }

    /*if(xbox.GetRawButton(2) && !_channel.Stop()){
        if(!_intake.Deployed()){
            _intake.Deploy();
        }
        _intake.Run();
        _channel.Run();
    }
    _intake.Retract();*/
    
    /*if(xbox.GetRawButton(3)){
        _climb.Extend();
    }
    */
}

void Controls::Auto(){
    _drivetrain.Auto();
}

void Controls::Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);
    _shooter.Manual(xbox);

    //TODO: Intake & Channel, Climber
}

void Controls::Testing(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    //_shooter.Calibration();
}