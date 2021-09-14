#include "Controls.h"
#include "Shoot.h"

void Controls::Periodic(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    drivetrain.Periodic(l_joy, r_joy);

    if(xbox.GetRawButton(1)){
        shooter.Periodic(xbox);
    }
    if (xbox.GetRawButton(6)) shooter.setState(Shoot::State::Aiming);
}

void Controls::Manual(const frc::XboxController & xbox, const frc::Joystick & l_joy, const frc::Joystick & r_joy){
    drivetrain.Periodic(l_joy, r_joy);
    shooter.Manual(xbox);
}