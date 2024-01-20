#include <cll/event_compressor.h>

#define ZLIB_CONST
#include <zlib.h>
#include <stdexcept>

using namespace cll;

std::string EventCompressor::compress(const char* data, size_t size) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    int ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        throw CompressError("Failed to call deflateInit");
    zs.avail_in = (uInt) size;
    zs.next_in = (unsigned char*) data;
    std::string out;
    do {
        out.resize(out.size() + 4096);
        zs.next_out = (unsigned char*) &out[out.size() - 4096];
        zs.avail_out = 4096;
        ret = deflate(&zs, Z_FINISH);
        if (zs.avail_out != 0)
            out.resize(out.size() - zs.avail_out);
        if (ret == Z_STREAM_ERROR)
            throw CompressError("deflate error");
    } while (zs.avail_out == 0);
    deflateEnd(&zs);
    return std::move(out);
}

std::string EventCompressor::decompress(const char* data, size_t size) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    int ret = inflateInit2(&zs, -15);
    if (ret != Z_OK)
        throw CompressError("Failed to call inflateInit");
    zs.avail_in = (uInt) size;
    zs.next_in = (unsigned char*) data;
    std::string out;
    do {
        out.resize(out.size() + 4096);
        zs.next_out = (unsigned char*) &out[out.size() - 4096];
        zs.avail_out = 4096;
        ret = inflate(&zs, Z_FINISH);
        if (zs.avail_out != 0)
            out.resize(out.size() - zs.avail_out);
        if (ret == Z_STREAM_ERROR)
            throw CompressError("inflate error");
    } while (zs.avail_out == 0);
    inflateEnd(&zs);
    return std::move(out);
}