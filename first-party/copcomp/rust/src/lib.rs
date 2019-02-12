use serde::{Deserialize, Serialize};
use serde_cbor;
use std::borrow::{Borrow, BorrowMut};
use std::io;
use std::io::Cursor;
use std::net::UdpSocket;
#[macro_use]
extern crate serde_derive;

pub mod c2019;

#[derive(Debug)]
pub enum Error {
    CopComp(ErrorKind),
    Io(io::Error),
    Cbor(serde_cbor::error::Error),
}

#[derive(Debug, Copy, Clone)]
pub enum ErrorKind {
    NegotationFailed,
    ReaderConsumed,
}

impl From<io::Error> for Error {
    fn from(error: io::Error) -> Error {
        Error::Io(error)
    }
}

impl From<serde_cbor::error::Error> for Error {
    fn from(error: serde_cbor::error::Error) -> Error {
        Error::Cbor(error)
    }
}

pub struct Connection {
    udp: UdpSocket,
    data: Box<[u8]>,
}

pub type Result<T> = std::result::Result<T, Error>;

use std::time::Duration;
impl Connection {
    const BUF_LEN: usize = 64 * 1024;
    pub fn from_udp(udp: UdpSocket, rt: Option<Duration>, wt: Option<Duration>) -> Result<Self> {
        udp.set_read_timeout(rt)?;
        udp.set_write_timeout(wt)?;

        Ok(Connection {
            udp: udp,
            data: vec![0u8; Self::BUF_LEN].into_boxed_slice(),
        })
    }

    pub fn write_item<W: Serialize>(&mut self, item: &W) -> Result<()> {
        let slice: &mut [u8] = self.data.borrow_mut();
        let mut cursor = Cursor::new(slice);
        serde_cbor::to_writer(&mut cursor, item)?;
        let idx = cursor.position() as usize;
        let slice: &[u8] = self.data.borrow();
        self.udp.send(&slice[..idx])?;
        Ok(())
    }

    pub fn read_item<R>(&mut self) -> Result<R>
    where
        R: for<'de> Deserialize<'de>,
    {
        let bytes = self.udp.recv(self.data.borrow_mut())?;
        let slice: &[u8] = self.data.borrow();
        let result: R = serde_cbor::from_reader(&slice[..bytes])?;
        Ok(result)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[derive(Debug, Serialize, Deserialize, Default, Clone, PartialEq, Eq)]
    struct Packet {
        idx: u64,
        msg: Option<i64>,
        t: [u8; 10],
    }

    use std::thread;
    #[test]
    fn basic_function() {
        let packet = Packet {
            idx: 617,
            msg: Some(-128309),
            t: [6, 54, 243, 6, 54, 243, 13, 6, 7, 32],
        };
        let packet2 = Packet {
            idx: 12303840932948,
            msg: None,
            t: [6, 54, 243, 234, 0, 243, 121, 6, 7, 32],
        };
        let ppacket = packet.clone();
        let ppacket2 = packet2.clone();

        let listener = UdpSocket::bind("0.0.0.0:5808").unwrap();
        thread::spawn(move || {
            let sender = UdpSocket::bind("0.0.0.0:5809").unwrap();
            sender.connect("0.0.0.0:5808").unwrap();
            let mut con = Connection::from_udp(sender, None, None).unwrap();
            con.write_item(&ppacket).unwrap();
            con.write_item(&ppacket2).unwrap();
        });
        let mut con =
            Connection::from_udp(listener, Some(std::time::Duration::from_millis(5)), None)
                .unwrap();

        assert_eq!(con.read_item::<Packet>().unwrap(), packet);
        assert_eq!(con.read_item::<Packet>().unwrap(), packet2);
    }
}
