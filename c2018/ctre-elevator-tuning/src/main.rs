use wpilib::RobotBase;
extern crate ctre_elevator_tuning;
use ctre_elevator_tuning::Elevator;
const IS_USING_MP: bool = true;
fn main() {
    let base = RobotBase::new().expect("HAL");
    let comp = wpilib::pneumatics::Compressor::new().unwrap();
    comp.stop();

    let ds = base.make_ds();
    let joy = wpilib::ds::JoystickPort::new(0).unwrap();
    let axis = wpilib::ds::JoystickAxis::new(1).unwrap();
    let mut elev = Elevator::new().expect("ELEVATOR");
    RobotBase::start_competition();
    loop {
        match ds.robot_state() {
            wpilib::ds::RobotState::Disabled => {
                continue;
            }
            _ => {}
        }
        // this is faster than joy input, but elevator iteration is 200hz
        std::thread::sleep(std::time::Duration::from_millis(5));

        if IS_USING_MP {
            let sp = (ds.stick_axis(joy, axis).unwrap_or(0.0) * -1.0 / 2.0 + 0.5) as f64
                * Elevator::MAX_HEIGHT;
            elev.set_goal(sp);
            println!("sp {}", sp);
            elev.iterate().expect("ITER FAILED");
        } else {
            let cmd = ds.stick_axis(joy, axis).unwrap_or(0.0) * -1.0;
            let vel = elev.set_fff_cmd(cmd.into()).unwrap();
            println!("cmd {:.2}, vel {}", cmd, vel);
            println!("cmd {}", cmd);
            // DONT ITERATE THE ELEVATOR!
        }
    }
}
