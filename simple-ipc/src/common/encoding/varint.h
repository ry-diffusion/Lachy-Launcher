#pragma once

#include <cstddef>

namespace simpleipc {
namespace encoding {

class varint {

public:
    static void encode_unsigned(unsigned long long i, char out[10], size_t& outs);

    static bool try_decode_unsigned(const char* in, size_t ins, unsigned long long& res, size_t* len = nullptr);

    static unsigned long long decode_unsigned(const char* in, size_t ins, size_t* len = nullptr);

};

}
}