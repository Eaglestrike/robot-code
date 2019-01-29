//! Module contains configuration variables.
//! These should use `dimensioned` units wherever applicable, and contain units in the name otherwise.
use std::time::Duration;
pub const SUBSYSTEM_SLEEP_TIME: Duration = Duration::from_millis(5);

/// The number of messages a broadcast bus can hold before stalling. Each subsystem contains one.
pub const BUS_SIZE: usize = 1024;

pub mod drive {
    /// ID of the master talon on the left side of the robot
    pub const LEFT_MASTER: i32 = -1; //TODO

    /// ID of the slave talon on the left side of the robot
    pub const LEFT_SLAVE: i32 = -1; //TODO

    /// ID of the master talon on the right side of the robot
    pub const RIGHT_MASTER: i32 = -1; //TODO

    /// ID of the slave talon on the right side of the robot
    pub const RIGHT_SLAVE: i32 = -1; //TODO

    /// The number of meters per tick of the drive encoders
    pub const ENCODER_METERS_PER_TICK: f64 = -1.0; //TODO

    /// Maximum allowed velocity when being controlled by the operator
    pub const MAX_VELOCITY: f64 = 6.0;

    /// Maximum allowed acceleration when being controlled by the operator
    pub const MAX_ACCELERATION: f64 = 4.0;

    /// Distance between the wheels on each drive side. This value will be tweaked later when we do
    /// tests for calibration which will account for wheel skid.
    pub const DRIVE_BASE_WHEEL_WIDTH: f64 = -1.0;

    //TODO: What exactly does this do?
    /// Something related to subsystem.drive PID constants, but I honestly have no idea what this does.
    pub const LOW_GEAR_VEL_PID_IDX: i32 = 0;

    /// Maximum current allowed before disabling the talon. Units are in amps.
    pub const CURRENT_LIMIT_THRESHOLD: i32 = 60;

    /// Limit for sustained current in the motor. Units are in amps.
    pub const CURRENT_LIMIT: i32 = 50;

    /// Limit for duration of sustained current
    pub const CURRENT_LIMIT_DURATION_MS: i32 = 200;

    /// Communication timeout for setting the talon configurations
    pub const TALON_CONFIG_TIMEOUT_MS: i32 = 10;

    /// Gear shifter for the drive base.
    pub mod shifter {
        /// Gear shifter solenoid channel ID for high gear
        pub const HIGH_GEAR_CHANNEL: i32 = 0;

        /// Gear shifter solenoid channel ID for low gear
        pub const LOW_GEAR_CHANNEL: i32 = 1;

        /// Ratio of the high gear speed to the low gear speed
        pub const HIGH_LOW_GEAR_RATIO: f64 = 0.0;
    }
}
