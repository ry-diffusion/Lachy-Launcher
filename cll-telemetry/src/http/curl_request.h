#pragma once

#include <string>
#include <curl/curl.h>
#include <cll/http/http_request.h>

namespace cll {
namespace http {

class CurlHttpRequest : public HttpRequest {

private:
    CURL* curl;
    struct curl_slist* headers = nullptr;

    static std::function<void (CURL* curl)> platformCurlInitHook;

    static size_t curlHeaderHandler(char* buffer, size_t size, size_t nitems, HttpResponse* data);

    static size_t curlOutputHandler(void* ptr, size_t size, size_t nmemb, std::ostream* s);

    static int curlDebugCallback(CURL* handle, curl_infotype type, char *data, size_t size, void* userptr);

public:
    static void setPlatformCurlInitHook(std::function<void (CURL* curl)> func);

    CurlHttpRequest();

    ~CurlHttpRequest() override;

    void setUrl(std::string const& url) override;

    void setMethod(HttpMethod method) override;

    void setPostData(const char* data, size_t size) override;

    void addHeader(std::string const& name, std::string const& value) override;

    HttpResponse send() override;

};

}
}