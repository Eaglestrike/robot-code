//! Module contains configuration variables.
//! These should use `dimensioned` units wherever applicable, and contain units in the name otherwise.

pub mod dashboard {
    /// Openmct websocket address
    pub const OPENMCT_URL: &str = "ws://localhost:8080/update";

    /// Milliseconds between sending updates to the dashboard
    pub const UPDATE_INTERVAL_MS: u64 = 80;

    /// Milliseconds without a valid response before restarting the web socket
    pub const PACKET_TIMEOUT: u64 = 200;
}
