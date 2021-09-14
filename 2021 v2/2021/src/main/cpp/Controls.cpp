#include "Controls.h"

void Controls::Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);

    if(xbox.GetRawButton(1)){
        _shooter.Periodic(xbox);
    }
}

void Controls::Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    _drivetrain.Periodic(l_joy, r_joy);
    _shooter.Manual(xbox);
}