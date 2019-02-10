# Protocol

Connection is two-way, after TCP is established the protocol is server-client agnostic. "Client" will be used to refer to either
member of the data exchange.

On sucessful connection establishment, each connection must send the 4-byte ASCII string `!114`, followed
by one byte containing the protocol version as an unsigned integer. Versioning begins at `0x01`; `0x00` is reserved.

Once a client has sent the above header and verified the received protocol version matches their own, they may begin sending datagrams.

## Version 1

After protocol negotation, the data stream is a sequence of CBOR items, as described in [Section 3.1 of the RFC](https://tools.ietf.org/html/rfc7049#section-3.1).
