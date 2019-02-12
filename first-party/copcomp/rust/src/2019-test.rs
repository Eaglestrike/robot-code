use copcomp::c2019::Packet;
use copcomp::Connection;
use std::net::UdpSocket;

fn main() {
    let listener = UdpSocket::bind("0.0.0.0:5808").unwrap();
    let mut con = Connection::from_udp(listener, None, None).unwrap();
    loop {
        println!("{:#?}", con.read_item::<Packet>());
    }
}
