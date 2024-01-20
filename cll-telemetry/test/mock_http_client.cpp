#include <cll/http/mock_http_client.h>
#include <gtest/gtest.h>

using namespace cll::http;

TEST(MockHttpClientTest, BasicTest) {
    MockHttpClient client;
    HttpResponse resp = {200, {{"Header", "Value"}}, "This is an example request body"};
    client.addMockedResponse("http://example.com/", resp);
    auto req = client.createRequest();
    req->setUrl("http://example.com/");
    auto resp2 = req->send();
    ASSERT_EQ(resp2.status, resp.status);
    ASSERT_EQ(resp2.headers, resp.headers);
    ASSERT_EQ(resp2.body, resp.body);
}

TEST(MockHttpClientTest, NotFoundTest) {
    MockHttpClient client;
    auto req = client.createRequest();
    req->setUrl("http://example.com/");
    auto resp = req->send();
    ASSERT_EQ(resp.status, 404);
}