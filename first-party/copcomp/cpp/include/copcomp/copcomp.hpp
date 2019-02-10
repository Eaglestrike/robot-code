#include <cstdint>
#include <inetclientstream.hpp>

namespace team114
{
namespace copcomp
{

const uint8_t NEG_BUF[5] = {0x21, 0x31, 0x31, 0x34, 0x01};

class Connection
{
  public:
    Connection(const std::string &dsthost, const std::string &dstport);
    void Negotiate();
    template <typename T> void write_item(const T &item)
    {
        Negotiate();
        auto fd = tcp.getfd();
    }
    template <typename T> T recv_item();

  private:
    enum class Negotiation {
        SUCCESS,
        FAILED,
        SENT,
        UNINIT,
    };
    libsocket::inet_stream tcp;
    Negotiation status;
    uint8_t neg_buf[5];
};

} // namespace copcomp
} // namespace team114
