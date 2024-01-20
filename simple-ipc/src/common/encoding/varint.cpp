#include "varint.h"

#include <stdexcept>

using namespace simpleipc::encoding;

void varint::encode_unsigned(unsigned long long i, char* out, size_t& outs) {
    outs = 0;
    do {
        out[outs++] = (char) ((unsigned char) i | (1 << 7));
        i >>= 7;
    } while (i > 0);
    out[outs - 1] &= 0b01111111;
}

bool varint::try_decode_unsigned(const char* in, size_t ins, unsigned long long& res, size_t* len) {
    unsigned long long tmp = 0;
    unsigned char b;
    if (ins > 9)
        ins = 9;
    for (size_t o = 0; o < ins; o++) {
        b = (unsigned char) in[o];
        tmp = tmp | ((b & 127) << (o * 7));
        if (!(b >> 7)) {
            if (len)
                *len = o + 1;
            res = tmp;
            return true;
        }
    }
    return false;
}

unsigned long long varint::decode_unsigned(const char* in, size_t ins, size_t* len) {
    unsigned long long ret;
    if (!try_decode_unsigned(in, ins, ret, len))
        throw std::runtime_error("Invalid varint");
    return ret;
}