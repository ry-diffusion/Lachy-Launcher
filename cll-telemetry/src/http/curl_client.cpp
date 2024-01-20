#include "curl_client.h"
#include "curl_request.h"

using namespace cll::http;

std::unique_ptr<HttpRequest> CurlHttpClient::createRequest() {
    return std::unique_ptr<HttpRequest>(new CurlHttpRequest());
}


std::unique_ptr<HttpClient> HttpClient::createPlatformClient() {
    return std::unique_ptr<HttpClient>(new CurlHttpClient());
}
