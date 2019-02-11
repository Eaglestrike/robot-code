#include <assert.h>
#include <copcomp/copcomp.hpp>
#include <netdb.h>
#include <netinet/tcp.h>

// TODO research quickack to reduce latency:
// https://stackoverflow.com/questions/7286592/set-tcp-quickack-and-tcp-nodelay/13209750
// https://news.ycombinator.com/item?id=10608356
// https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_MRG/1.3/html/Realtime_Tuning_Guide/sect-Realtime_Tuning_Guide-General_System_Tuning-Reducing_the_TCP_delayed_ack_timeout.html

namespace team114
{
namespace copcomp
{

using namespace std;

Connection::Connection(const string &dsthost, const string &dstport)
    : tcp(dsthost, dstport, LIBSOCKET_IPv4, SOCK_NONBLOCK), status(Negotiation::UNINIT), neg_buf() // zero-initialize neg_buf
{
    protoent *tcp_db = getprotobyname("tcp");
    static_assert(IPPROTO_TCP == 6);
    assert(tcp_db->p_proto == 6); // IANA TCP protocol #;
    assert(tcp_db->p_proto == IPPROTO_TCP);
    int i = 1;
    tcp.set_sock_opt(tcp_db->p_proto, TCP_NODELAY, (char *)(&i), sizeof(i));
}

void Connection::Negotiate()
{
    switch (status) {
    case Negotiation::SUCCESS:
        return;
    case Negotiation::FAILED:
        // TODO
        throw 0;
    case Negotiation::SENT:
        // attempt to read 5 bytes
        tcp.rcv(neg_buf, sizeof(neg_buf), MSG_WAITALL);
        for (int i = 0; i < sizeof(neg_buf); i++) {
            if (neg_buf[i] != NEG_BUF[i]) {
                status = Negotiation::FAILED;
                return Negotiate();
            }
        }
        status = Negotiation::SUCCESS;
        return Negotiate();
    case Negotiation::UNINIT:
        tcp.snd(NEG_BUF, sizeof(NEG_BUF));
        status = Negotiation::SENT;
        return Negotiate();
    }
}

} // namespace copcomp
} // namespace team114
