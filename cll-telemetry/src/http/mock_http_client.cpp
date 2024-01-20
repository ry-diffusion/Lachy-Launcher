#include <cll/http/mock_http_client.h>
#include "mock_http_request.h"

using namespace cll::http;

HttpResponse MockHttpClient::getMockedResponse(std::string const& url) {
    auto ret = responses.find(url);
    if (ret != responses.end())
        return ret->second;
    HttpResponse fb;
    fb.status = 404;
    fb.body = "<!doctype html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found";
    fb.headers = {};
    return fb;
}

std::unique_ptr<HttpRequest> MockHttpClient::createRequest() {
    return std::unique_ptr<HttpRequest>(new MockHttpRequest(*this));
}