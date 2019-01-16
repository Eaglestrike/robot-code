/// Made from Maxwell and Leo's openmct-dashboard, but following the style guide
use crate::util::config::OPENMCT_URL;
use bus::BusReader;
use crossbeam_channel::{unbounded, Receiver, Sender};
use ws::{connect, CloseCode, Message::Binary};

use super::Subsystem;

/// Packet to serialize and send to the driver station
#[derive(Serialize, Deserialize, Debug)]
pub struct Packet<T> {
    name: String,
    value: T,
}

/// Instructions for what to send to the dashboard. This exists since I would have to generify
/// Dashboard over a single packet type otherwise. Once this is received, it can be converted to a
/// packet without issue.
pub enum Instruction {
    Integer(String, i32),
    Float(String, f64),
    String(String, String),
}

/// Dashboard subsystem which monitors the outputs of other subsystems and sends their output to
/// the dashboard accordingly. If you do not want to broadcast a specific piece of information for
/// all subsystems, sending a broadcast instruction is also an option.
pub struct Dashboard {
    instructions: Receiver<Instruction>,
}

impl Dashboard {
    pub fn new() -> (Self, Sender<Instruction>) {
        let (sender, instructions) = unbounded();
        (Dashboard { instructions }, sender)
    }
}

impl Subsystem<()> for Dashboard {
    fn run(&mut self) {
        connect(OPENMCT_URL, |out| {
            loop {
                // Read any instructions sent to the subsystem
                for item in self.instructions.try_iter() {
                    let p = match item {
                        Instruction::Integer(a, b) => {
                            serde_cbor::to_vec(&Packet { name: a, value: b })
                        }
                        Instruction::Float(a, b) => {
                            serde_cbor::to_vec(&Packet { name: a, value: b })
                        }
                        Instruction::String(a, b) => {
                            serde_cbor::to_vec(&Packet { name: a, value: b })
                        }
                    };
                    match p {
                        Ok(v) => out.send(Binary(v)).expect("Unable to sent to dashboard!"),
                        Err(_) => eprintln!("Got invalid packet!"),
                    }
                }

                // TODO: Send basic information!
            }
            // This is here to fulfill the closure return requirements
            #[allow(unreachable_code)]
            |_| out.close(CloseCode::Normal)
        })
        .unwrap();
    }

    fn create_receiver(&mut self) -> BusReader<()> {
        unimplemented!("The dashboard does not send back any information at this time!")
    }
}
