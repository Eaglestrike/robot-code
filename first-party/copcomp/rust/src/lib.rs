use serde::{Deserialize, Serialize};
use serde_cbor;
use serde_cbor::de::{Deserializer, IoRead, StreamDeserializer};
use std::io;
use std::io::{Read, Write};
use std::mem;
use std::net::*;
#[cfg(test)]
#[macro_use]
extern crate serde_derive;

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

#[derive(Copy, Clone, Debug)]
enum Negotiation {
    Success,
    Failed,
    Sent,
    Uninit,
}

pub struct Connection {
    status: Negotiation,
    read: Option<TcpStream>,
    write: TcpStream,
    neg_buf: [u8; 5],
}

pub type Result<T> = std::result::Result<T, Error>;

use std::time::Duration;
impl Connection {
    const NEG_BUF: [u8; 5] = [0x21, 0x31, 0x31, 0x34, 0x01];
    pub fn from_tcp(tcp: TcpStream, rt: Option<Duration>, wt: Option<Duration>) -> Result<Self> {
        tcp.set_nodelay(true)?;
        tcp.set_read_timeout(rt)?;
        tcp.set_write_timeout(wt)?;

        Ok(Connection {
            status: Negotiation::Uninit,
            neg_buf: [0; 5],
            read: Some(tcp.try_clone()?),
            write: tcp,
        })
    }

    pub fn negotiate(&mut self) -> Result<()> {
        dbg!(self.status);
        match self.status {
            Negotiation::Success => {
                return Ok(());
            }
            Negotiation::Failed => {
                return Err(Error::CopComp(ErrorKind::NegotationFailed));
            }
            Negotiation::Sent => {
                if let Some(ref mut r) = self.read {
                    r.read_exact(&mut self.neg_buf[..])?;
                    if self.neg_buf != Self::NEG_BUF {
                        self.status = Negotiation::Failed;
                    } else {
                        self.status = Negotiation::Success;
                    }
                    return self.negotiate();
                } else {
                    return Err(Error::CopComp(ErrorKind::ReaderConsumed));
                }
            }
            Negotiation::Uninit => {
                if self.write.write_all(&Self::NEG_BUF).is_err() {
                    self.status = Negotiation::Failed;
                    return self.negotiate();
                }
                self.status = Negotiation::Sent;
                return self.negotiate();
            }
        }
    }

    pub fn write_item<W: Serialize>(&mut self, item: &W) -> Result<()> {
        self.negotiate()?;
        serde_cbor::to_writer(&mut self.write, item)?;
        Ok(())
    }

    pub fn make_iter<'de, R: Deserialize<'de>>(&mut self) -> Result<ReadIter<'de, R>> {
        self.negotiate()?;
        match mem::replace(&mut self.read, None) {
            Some(tcp) => Ok(Deserializer::from_reader(tcp).into_iter()),
            None => Err(Error::CopComp(ErrorKind::ReaderConsumed)),
        }
    }
}
pub type ReadIter<'de, T> = StreamDeserializer<'de, IoRead<TcpStream>, T>;

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
        let packet2 = packet.clone();

        let listener = TcpListener::bind("0.0.0.0:5808").unwrap();
        thread::spawn(move || {
            let stream = TcpStream::connect("0.0.0.0:5808").unwrap();
            let mut con = Connection::from_tcp(stream, None, None).unwrap();
            con.negotiate().unwrap();
            con.write_item(&packet2)
        });
        let (stream, _addr) = listener.accept().unwrap();
        let mut con = Connection::from_tcp(stream, None, None).unwrap();
        con.negotiate().unwrap();
        con.write_item(&packet).unwrap();
        let mut read_iter: ReadIter<'_, Packet> = con.make_iter().unwrap();
        assert_eq!(read_iter.next().unwrap().unwrap(), packet);
    }
}
