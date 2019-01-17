/*
 * UNITS
 *    angles: RADIANS
 *    distance: METERS
 *    time: SECONDS
 *
 *  YOU MUST USE THE CORRECT UNITS UNLESS OTHERWISE SPECIFIED IN VARIABLE NAME!
 */

/// The number of messages a broadcast bus can hold before stalling. Each subsystem contains one.
pub const BUS_SIZE: usize = 1024;

use std::time::Duration;
/// Tick duration for most situations
pub const TICK_RATE: Duration = Duration::from_millis(5);

pub mod dashboard {
    /// Openmct websocket address
    pub const OPENMCT_URL: &str = "ws://localhost:8080/update";

    /// Milliseconds between sending updates to the dashboard
    pub const PACKET_RATE: u64 = 80;

    /// Milliseconds without a valid response before restarting the web socket
    pub const PACKET_TIMEOUT: u64 = 200;
}

pub mod pneumatics {

    ///
    pub const TYPICAL_PNEUMATIC_SUPPLY_VOLTAGE: f64 = 5.0;

    /// Channel ID of the pneumatics compressor
    pub const PNEUMATIC_PRESSURE_SENSOR_ID: i32 = 0;

    ///
    pub const DEFAULT_PRESSURE_MARGIN: f64 = 60.0;

    ///
    pub const DEFAULT_ACTIVATION_PRESSURE: f64 = -1.0;
}

/// Settings for controllers
pub mod controls {

    /// Dead band of controller input to the robot.
    pub const STANDARD_DEADBAND: f64 = 0.2;

    /// Dead band to exit a control state.
    pub const FREER_DEADBAND: f64 = 0.5;
}

pub mod drive {
    /// ID of the master talon on the left side of the robot
    pub const LEFT_MASTER: i32 = -1;

    /// ID of the slave talon on the left side of the robot
    pub const LEFT_SLAVE: i32 = -1;

    /// ID of the master talon on the right side of the robot
    pub const RIGHT_MASTER: i32 = -1;

    /// ID of the slave talon on the right side of the robot
    pub const RIGHT_SLAVE: i32 = -1;

    /// The number of meters per tick of the drive encoders
    pub const ENCODER_METERS_PER_TICK: f64 = -1.0;

    /// Maximum allowed velocity when being controlled by the operator
    pub const MAX_VELOCITY: f64 = 6.0;

    /// Maximum allowed acceleration when being controlled by the operator
    pub const MAX_ACCELERATION: f64 = 4.0;

    /// Distance between the wheels on each drive side. This value will be tweaked later when we do
    /// tests for calibration which will account for wheel skid.
    pub const DRIVE_BASE_WHEEL_WIDTH: f64 = -1.0;

    /// Talon specific settings
    pub mod talon {
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
    }

    /// PWM specific settings
    pub mod pwm {
        pub const LEFT_ENCODER_A: i32 = 0;
        pub const LEFT_ENCODER_B: i32 = 0;
        pub const RIGHT_ENCODER_A: i32 = 0;
        pub const RIGHT_ENCODER_B: i32 = 0;

        pub const LEFT_K_VELOCITY: f64 = 0.0;
        pub const LEFT_K_ACCELERATION: f64 = 0.0;
        pub const RIGHT_K_VELOCITY: f64 = 0.0;
        pub const RIGHT_K_ACCELERATION: f64 = 0.0;
    }

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
