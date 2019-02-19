use wpilib::RobotBase;
extern crate ctre_elevator_tuning;
use ctre_elevator_tuning::Elevator;
fn main() {
    let base = RobotBase::new().expect("HAL");

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
        std::thread::sleep(std::time::Duration::from_millis(50));
        let sp = (ds.stick_axis(joy, axis).unwrap_or(0.0) * -1.0 / 2.0 + 0.5) as f64
            * Elevator::MAX_HEIGHT;
        elev.set_goal(sp);
        println!("sp {}", sp);
        elev.iterate().expect("ITER FAILED");
        println!("{:?}", elev.state());
    }
}
