use crossbeam_channel::{unbounded, Receiver, Sender};
use ws::util::{Timeout, Token};
use ws::Sender as WsSender;
use ws::{connect, CloseCode, Handler, Handshake, Message, Result};

use serde_derive::{Deserialize, Serialize};

/// Made from Maxwell and Leo's openmct-dashboard, but following the style guide
use crate::config::dashboard as config;

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
#[derive(Debug, Clone)]
pub enum Instruction {
    Number(String, f64),
    String(String, String),
}

impl From<Instruction> for Message {
    fn from(raw_message: Instruction) -> Self {
        Message::Binary(
            match raw_message {
                Instruction::Number(a, b) => serde_cbor::to_vec(&Packet { name: a, value: b }),
                Instruction::String(a, b) => serde_cbor::to_vec(&Packet { name: a, value: b }),
            }
            .expect("Unable to serialize packet"),
        )
    }
}

const UPDATE: Token = Token(1);
const TIMEOUT: Token = Token(2);

struct DashboardHandle {
    web_socket: WsSender,
    instructions: Receiver<Instruction>,
    socket_timeout: Option<Timeout>,
}

impl Handler for DashboardHandle {
    fn on_open(&mut self, _: Handshake) -> Result<()> {
        self.web_socket.timeout(config::PACKET_RATE, UPDATE)?;
        self.web_socket.timeout(config::PACKET_TIMEOUT, TIMEOUT)
    }

    // Timeout if a "Success" response has not received in X ms
    fn on_message(&mut self, msg: Message) -> Result<()> {
        // TODO: Is this valid?
        if msg == Message::Text("Success".to_string()) {
            self.socket_timeout.take();
        }
        Ok(())
    }

    fn on_timeout(&mut self, event: Token) -> Result<()> {
        match event {
            UPDATE => {
                // Send any instructions
                for item in self.instructions.try_iter() {
                    self.web_socket.send(Message::from(item))?;
                }

                //TODO: Figure out what information will be on the dashboard and how it gets here

                self.web_socket.timeout(config::PACKET_RATE, UPDATE)
            }
            TIMEOUT => self.web_socket.close(CloseCode::Away),
            _ => Err(ws::Error::new(
                ws::ErrorKind::Internal,
                "Invalid timeout token encountered!",
            )),
        }
    }

    fn on_new_timeout(&mut self, event: Token, timeout: Timeout) -> Result<()> {
        if event == TIMEOUT {
            if let Some(t) = self.socket_timeout.take() {
                self.web_socket.cancel(t)?
            }
            self.socket_timeout = Some(timeout)
        }
        Ok(())
    }
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

impl Subsystem for Dashboard {
    fn run(&mut self) {
        let mut timeouts = 0;
        loop {
            connect(config::OPENMCT_URL, |web_socket| DashboardHandle {
                web_socket,
                instructions: self.instructions.clone(),
                socket_timeout: None,
            })
            .unwrap();
            timeouts += 1;

            if timeouts % 5 == 0 {
                eprintln!(
                    "Dashboard websocket timed out, check that the server was started \
                     correctly! Timeout count: {:?} ",
                    timeouts
                );
            }
        }
    }
}
