#include <cll/configuration.h>
#include <gtest/gtest.h>
#include <cll/http/mock_http_client.h>

using namespace cll;
using namespace cll::http;

TEST(ConfigurationTest, Download) {
    Configuration config ("http://localhost/config.json");
    EXPECT_TRUE(config.needsRedownload());
    MockHttpClient httpClient;
    httpClient.addMockedResponse("http://localhost/config.json", {200, {}, "{\"refreshInterval\":\"12\",\"settings\":{\"MAXEVENTSIZEINBYTES\":\"1234\",\"MAXEVENTSPERPOST\":\"5678\"}}"});
    EXPECT_TRUE(config.download(httpClient, nullptr));
    EXPECT_EQ(config.maxEventSizeInBytes.get(), 1234);
    EXPECT_EQ(config.maxEventsPerPost.get(), 5678);
    ASSERT_FALSE(config.needsRedownload());
}
