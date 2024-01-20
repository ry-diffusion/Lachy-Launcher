#include <cll/event_compressor.h>
#include <gtest/gtest.h>
#include <random>

using namespace cll;

TEST(EventCompressorTest, BasicTest) {
    std::string data;
    data.resize(4096 * 10 + 17);
    std::mt19937 rand (645634127);
    std::independent_bits_engine<std::mt19937, 8, unsigned char> engine (rand);
    std::generate(data.begin(), data.end(), engine);

    std::string compressed = EventCompressor::compress(data.data(), data.size());
    std::string decompressed = EventCompressor::decompress(compressed.data(), compressed.size());
    ASSERT_EQ(data, decompressed);
}