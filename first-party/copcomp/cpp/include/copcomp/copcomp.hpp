#include <array>
#include <cstdint>
#include <inetclientdgram.hpp>

namespace team114
{
namespace copcomp
{

class Connection
{
  public:
    Connection(const std::string &dsthost, const std::string &dstport);
    virtual ~Connection();
    template <typename T> void write_item(const T &item)
    {
        size_t bytes = item.cbor_serialize(data, BUFFER_LEN);
        udp.snd(data, bytes);
    }

    // template <typename T> void write_item_to(const T &item, std::string &dsthost, std::string &dstport)
    // {
    //     size_t bytes = item.cbor_serialize(data, BUFFER_LEN);
    //     udp.sndto(data, bytes, dsthost, dstport);
    // }

    template <typename T> T recv_item()
    {
        size_t bytes = udp.rcv(data, BUFFER_LEN);
        T t = T::cbor_deserialize(data, bytes);
        return t;
    }

    // template <typename T> T recv_item_from(std::string &srchost, std::string &srcport)
    // {
    //     size_t bytes = udp.rcvfrom(data, BUFFER_LEN, srchost, srcport);
    //     T t = T::cbor_deserialize(data, bytes);
    //     return t;
    // }

  private:
    static constexpr size_t BUFFER_LEN = 65 * 1024 * 1024;
    libsocket::inet_dgram_client udp;
    uint8_t *data; // enough to store the max UDP packet size
};

} // namespace copcomp
} // namespace team114
