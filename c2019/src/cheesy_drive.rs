/// "Cheesy Drive" simply means that the "turning" stick controls the curvature
/// of the robot's path rather than its rate of heading change. This helps make the robot more controllable at high
/// speeds. Also handles the robot's quick turn functionality - "quick turn" overrides constant-curvature turning for
/// turn-in-place maneuvers.
// Translated from code released by FRC254 under the MIT license
use controls::util::clamp;
use std::f64::consts::PI;

const THROTTLE_DEADBAND: f64 = 0.02;
const WHEEL_DEADBAND: f64 = 0.02;

// These factor determine how fast the wheel traverses the "non linear" sine curve.
const HIGH_WHEEL_NON_LINEARITY: f64 = 0.65;
const LOW_WHEEL_NON_LINEARITY: f64 = 0.5;

const HIGH_NEG_INERTIA_SCALAR: f64 = 4.0;

const LOW_NEG_INERTIA_THRESHOLD: f64 = 0.65;
const LOW_NEG_INERTIA_TURN_SCALAR: f64 = 3.5;
const LOW_NEG_INERTIA_CLOSE_SCALAR: f64 = 4.0;
const LOW_NEG_INERTIA_FAR_SCALAR: f64 = 5.0;

const HIGH_SENSITIVITY: f64 = 0.65;
const LOW_SENSITIITY: f64 = 0.65;

const QUICSTOP_DEADBAND: f64 = 0.5;
const QUICSTOP_WEIGHT: f64 = 0.1;
const QUICSTOP_SCALAR: f64 = 5.0;

#[derive(Debug, Copy, Clone)]
pub struct DriveSignal {
    pub l: f64,
    pub r: f64,
}

#[derive(Debug)]
pub struct CheesyDrive {
    old_wheel: f64,
    quick_stop_accumlator: f64,
    neg_inertia_accumlator: f64,
}

impl CheesyDrive {
    pub fn new() -> Self {
        Self {
            old_wheel: 0.0,
            quick_stop_accumlator: 0.0,
            neg_inertia_accumlator: 0.0,
        }
    }

    pub fn cheesy_drive(
        &mut self,
        throttle: f64,
        wheel: f64,
        is_quick_turn: bool,
        is_high_gear: bool,
    ) -> DriveSignal {
        let mut wheel = handle_deadband(wheel, WHEEL_DEADBAND);
        let throttle = handle_deadband(throttle, THROTTLE_DEADBAND);

        let neg_inertia = wheel - self.old_wheel;
        self.old_wheel = wheel;

        let wheel_non_linearity;
        if is_high_gear {
            wheel_non_linearity = HIGH_WHEEL_NON_LINEARITY;
            let denominator = f64::sin(PI / 2.0 * wheel_non_linearity);
            // Apply a sin function that's scaled to make it feel better.
            wheel = f64::sin(PI / 2.0 * wheel_non_linearity * wheel) / denominator;
            wheel = f64::sin(PI / 2.0 * wheel_non_linearity * wheel) / denominator;
        } else {
            wheel_non_linearity = LOW_WHEEL_NON_LINEARITY;
            let denominator = f64::sin(PI / 2.0 * wheel_non_linearity);
            // Apply a sin function that's scaled to make it feel better.
            wheel = f64::sin(PI / 2.0 * wheel_non_linearity * wheel) / denominator;
            wheel = f64::sin(PI / 2.0 * wheel_non_linearity * wheel) / denominator;
            wheel = f64::sin(PI / 2.0 * wheel_non_linearity * wheel) / denominator;
        }

        let over_power;
        let sensitivity;

        let angular_power;
        let linear_power;

        // Negative inertia!
        let neg_inertia_scalar;
        if is_high_gear {
            neg_inertia_scalar = HIGH_NEG_INERTIA_SCALAR;
            sensitivity = HIGH_SENSITIVITY;
        } else {
            if wheel * neg_inertia > 0.0 {
                // If we are moving away from 0.0, aka, trying to get more wheel.
                neg_inertia_scalar = LOW_NEG_INERTIA_TURN_SCALAR;
            } else {
                // Otherwise, we areDriveSignal attempting to go back to 0.0.
                if f64::abs(wheel) > LOW_NEG_INERTIA_THRESHOLD {
                    neg_inertia_scalar = LOW_NEG_INERTIA_FAR_SCALAR;
                } else {
                    neg_inertia_scalar = LOW_NEG_INERTIA_CLOSE_SCALAR;
                }
            }
            sensitivity = LOW_SENSITIITY;
        }
        let neg_inertia_power = neg_inertia * neg_inertia_scalar;
        self.neg_inertia_accumlator += neg_inertia_power;

        wheel += self.neg_inertia_accumlator;
        if self.neg_inertia_accumlator > 1.0 {
            self.neg_inertia_accumlator -= 1.0;
        } else if self.neg_inertia_accumlator < -1.0 {
            self.neg_inertia_accumlator += 1.0;
        } else {
            self.neg_inertia_accumlator = 0.0;
        }
        linear_power = throttle;

        // Quickturn!
        if is_quick_turn {
            if f64::abs(linear_power) < QUICSTOP_DEADBAND {
                let alpha = QUICSTOP_WEIGHT;
                self.quick_stop_accumlator = (1.0 - alpha) * self.quick_stop_accumlator
                    + alpha * clamp(wheel, -1.0, 1.0) * QUICSTOP_SCALAR;
            }
            over_power = 1.0;
            angular_power = wheel;
        } else {
            over_power = 0.0;
            angular_power = f64::abs(throttle) * wheel * sensitivity - self.quick_stop_accumlator;
            if self.quick_stop_accumlator > 1.0 {
                self.quick_stop_accumlator -= 1.0;
            } else if self.quick_stop_accumlator < -1.0 {
                self.quick_stop_accumlator += 1.0;
            } else {
                self.quick_stop_accumlator = 0.0;
            }
        }

        let mut right_pwm = linear_power;
        let mut left_pwm = linear_power;
        left_pwm += angular_power;
        right_pwm -= angular_power;

        if left_pwm > 1.0 {
            right_pwm -= over_power * (left_pwm - 1.0);
            left_pwm = 1.0;
        } else if right_pwm > 1.0 {
            left_pwm -= over_power * (right_pwm - 1.0);
            right_pwm = 1.0;
        } else if left_pwm < -1.0 {
            right_pwm += over_power * (-1.0 - left_pwm);
            left_pwm = -1.0;
        } else if right_pwm < -1.0 {
            left_pwm += over_power * (-1.0 - right_pwm);
            right_pwm = -1.0;
        }
        DriveSignal {
            l: left_pwm,
            r: right_pwm,
        }
    }
}

fn handle_deadband(val: f64, deadband: f64) -> f64 {
    if val.abs() < deadband {
        0.0
    } else {
        val
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use controls::approx::*;

    fn tester(c: &mut CheesyDrive, t: f64, w: f64, q: bool, h: bool, l: f64, r: f64) {
        let sig = c.cheesy_drive(t, w, q, h);
        assert_abs_diff_eq!(sig.l, l);
        assert_abs_diff_eq!(sig.r, r);
    }

    #[test]
    fn extrema() {
        let mut d = CheesyDrive::new();
        let c = &mut d;
        tester(c, 1.0, 0.0, false, true, 1.0, 1.0);
        tester(c, -1.0, 0.0, false, true, -1.0, -1.0);

        tester(c, 1.0, 0.0, true, true, 1.0, 1.0);
        tester(c, -1.0, 0.0, true, true, -1.0, -1.0);

        tester(c, 1.0, 0.0, true, false, 1.0, 1.0);
        tester(c, -1.0, 0.0, true, false, -1.0, -1.0);

        tester(c, 1.0, 0.0, false, false, 1.0, 1.0);
        tester(c, -1.0, 0.0, false, false, -1.0, -1.0);

        tester(c, 0.0, 1.0, false, false, 0.0, 0.0);
        tester(c, 0.0, -1.0, false, false, 0.0, 0.0);
    }
}
