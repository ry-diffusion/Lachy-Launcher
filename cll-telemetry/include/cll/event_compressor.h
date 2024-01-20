#pragma once

#include <string>
#include <stdexcept>

namespace cll {

class CompressError : public std::runtime_error {

public:
    explicit CompressError(const char* msg) : runtime_error(msg) {}
    explicit CompressError(const std::string& msg) : runtime_error(msg) {}

};

class EventCompressor {

public:
    static std::string compress(const char* data, size_t size);

    static std::string decompress(const char* data, size_t size);

};

}