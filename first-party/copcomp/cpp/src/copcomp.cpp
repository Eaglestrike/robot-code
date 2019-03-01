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

Connection::Connection(const string &dsthost, const string &dstport) : udp(dsthost, dstport, LIBSOCKET_IPv4, SOCK_NONBLOCK), data()
{
    data = new uint8_t[BUFFER_LEN]();
    // could set timeout for the socket
    // timeval t;
    // udp.set_sock_opt(SOL_SOCKET, SO_RCVTIMEO, (char *)(&t), sizeof(t));
}

Connection::~Connection() { delete[] data; }

} // namespace copcomp
} // namespace team114
