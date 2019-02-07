use controls::units::*;

pub enum Instruction {
    SetElevatorHeight(Meter<f64>),
    BeginBallIntake,
    AbortBallIntake,
    BallOuttake,
    HatchIntake,
    HatchOuttake,
}
