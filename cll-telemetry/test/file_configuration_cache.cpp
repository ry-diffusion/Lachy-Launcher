#include <cll/file_configuration_cache.h>
#include <gtest/gtest.h>

using namespace cll;

class FileConfigurationCacheTest : public ::testing::Test {
public:
    FileConfigurationCache cache;

    FileConfigurationCacheTest() : cache("test_file") {}

    ~FileConfigurationCacheTest() {
        remove("test_file");
    }
};


TEST_F(FileConfigurationCacheTest, BasicTest) {
    CachedConfiguration info;
    info.expires = std::chrono::system_clock::now() + std::chrono::hours(1);
    info.etag = "1234";
    info.data = {{"test", "This is a test config"}};
    cache.writeConfigToCache("http://localhost/", info);
    CachedConfiguration read;
    ASSERT_TRUE(cache.readFromCache("http://localhost/", read));
    ASSERT_EQ(std::chrono::duration_cast<std::chrono::milliseconds>(info.expires.time_since_epoch()),
              std::chrono::duration_cast<std::chrono::milliseconds>(read.expires.time_since_epoch()));
    ASSERT_EQ(read.etag, info.etag);
    ASSERT_EQ(read.data, info.data);
}
