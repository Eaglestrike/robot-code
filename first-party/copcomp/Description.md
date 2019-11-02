# It's Simple

CBOR items over UDP. If you want more than 64k/packet, you're likely doing
something wrong.

The mapping of structures to CBOR items (eg map vs array) is whatever
`serde_cbor` says it is in the rust code.
