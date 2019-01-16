use bus::BusReader;

/// Macro for sending information on a bus which also includes a test mode to verify buffers are
/// not overflowing.
macro_rules! send_bus {
    ($bus:expr, $msg:expr) => {
        #[cfg(not(bus_tests))]
        $bus.broadcast($msg);

        #[cfg(bus_tests)]
        {
            if let Err(..) = $bus.try_broadcast($msg) {
                eprintln!(
                    "Drive buffer overflowed! Current message capacity: {:?}",
                    crate::util::config::BUS_SIZE
                );
            }
        }
    };
}

#[macro_use]
pub mod drive;
pub mod pneumatics;

/// A generic subsystem object. B is the enum type of information sent by the bus broadcaster for
/// this subsystem.
pub trait Subsystem<B> {
    /// Create a new instance of this subsystem, then return Self and the instruction sender.
    /// (number of arguments will vary depending on needs)
    //  fn new() -> (Self, Sender<T>);

    /// Signals the subsystem to start. Subsystems should not create their own threads as they
    /// will be provided.
    fn run(&mut self);

    /// Get a new BusReader for information broadcast by this subsystem.
    fn create_receiver(&mut self) -> BusReader<B>;
}
