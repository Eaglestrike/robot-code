#include <cstddef>
#include <cstdint>

#include <cbor.h>

namespace team114
{
namespace c2019
{
namespace vision
{

#define CBOR_CHCK(call)                                                                                                                    \
    {                                                                                                                                      \
        CborError err = call;                                                                                                              \
        if (err != CborNoError) {                                                                                                          \
            throw err;                                                                                                                     \
        }                                                                                                                                  \
    }

#define CBOR_VAL(call)                                                                                                                     \
    if (!call) {                                                                                                                           \
        throw CborError::CborErrorImproperValue;                                                                                           \
    }

struct Packet {
    int64_t micros;
    float x;
    float y;

    size_t cbor_serialize(uint8_t *buffer, size_t maxlen) const
    {
        CborEncoder encoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer, maxlen, 0);
        CBOR_CHCK(cbor_encoder_create_array(&encoder, &arrayEncoder, 3));
        CBOR_CHCK(cbor_encode_int(&arrayEncoder, this->micros));
        CBOR_CHCK(cbor_encode_float(&arrayEncoder, this->x));
        CBOR_CHCK(cbor_encode_float(&arrayEncoder, this->y));
        CBOR_CHCK(cbor_encoder_close_container(&encoder, &arrayEncoder));
        return cbor_encoder_get_buffer_size(&encoder, buffer);
    };
    static Packet cbor_deserialize(uint8_t *buffer, size_t datalen)
    {
        CborParser parser;
        CborValue value;
        Packet result;
        CBOR_CHCK(cbor_parser_init(buffer, datalen, 0, &parser, &value));

        CborValue inArray;
        if (!cbor_value_is_array(&value))
            throw CborError::CborErrorImproperValue;

        CBOR_CHCK(cbor_value_enter_container(&value, &inArray));

        CBOR_VAL(cbor_value_is_integer(&inArray));
        CBOR_CHCK(cbor_value_get_int64(&inArray, &(result.micros)));

        CBOR_CHCK(cbor_value_advance(&inArray));
        CBOR_VAL(cbor_value_is_float(&inArray));
        CBOR_CHCK(cbor_value_get_float(&inArray, &(result.x)));

        CBOR_CHCK(cbor_value_advance(&inArray));
        CBOR_VAL(cbor_value_is_float(&inArray));
        CBOR_CHCK(cbor_value_get_float(&inArray, &(result.y)));

        CBOR_CHCK(cbor_value_leave_container(&value, &inArray));
        return result;
    };
};

} // namespace vision
} // namespace c2019
} // namespace team114
