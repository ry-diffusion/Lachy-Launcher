#pragma once

#include <cll/http/http_request.h>
#include <cll/http/mock_http_client.h>

namespace cll {
namespace http {

class MockHttpClient;

class MockHttpRequest : public HttpRequest {

private:
    MockHttpClient& client;
    std::string url;

public:
    MockHttpRequest(MockHttpClient& client) : client(client) {}

    void setUrl(std::string const& url) override {
        this->url = url;
    }

    void setMethod(HttpMethod method) override {}

    void setPostData(const char* data, size_t size) override {}

    void addHeader(std::string const& name, std::string const& value) override {}

    HttpResponse send() override  {
        return client.getMockedResponse(url);
    }

};

}
}