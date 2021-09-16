#include "Controls.h"

void Controls::Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);
    if(xbox.GetRawButton(6)){
        _shooter.setState(Shoot::State::Aiming);
        _shooter.Periodic();
    } else {
        _shooter.setState(Shoot::State::Idle);
    }
}

void Controls::Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);
    _shooter.Manual(xbox);
}