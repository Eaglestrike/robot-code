# Protocol

Connection is two-way, after TCP is established the protocol is server-client agnostic. "Client" will be used to refer to either
member of the data exchange.

On sucessful connection establishment, each connection must send the 4-byte ASCII string `!114`, followed
by one byte containing the protocol version as an unsigned integer. Versioning begins at `0x01`; `0x00` is reserved.

Once a client has sent the above header and verified the received protocol version matches their own, they may begin sending datagrams.

## Version 1

Each datagram consists of the control character `r`, or `0x72`, followed by 2 bytes specifying the length of the following CBOR data.
After such many bytes of CBOR data, either a new control character is sent, or the TCP connection is closed.
Version 1 contains only one control character.
