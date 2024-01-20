#include "curl_request.h"
#include "curl_error.h"

#include <cstring>
#include <sstream>
#include <unistd.h>

using namespace cll::http;

std::function<void (CURL* curl)> CurlHttpRequest::platformCurlInitHook;

void CurlHttpRequest::setPlatformCurlInitHook(std::function<void(CURL *curl)> func) {
    platformCurlInitHook = func;
}

CurlHttpRequest::CurlHttpRequest() {
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 64L);

    if (platformCurlInitHook)
        platformCurlInitHook(curl);
}

CurlHttpRequest::~CurlHttpRequest() {
    if (headers != nullptr)
        curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void CurlHttpRequest::setUrl(std::string const& url) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

void CurlHttpRequest::setMethod(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            break;
        case HttpMethod::POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            break;
    }
}

void CurlHttpRequest::setPostData(const char* data, size_t size) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) size);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
}

void CurlHttpRequest::addHeader(std::string const& name, std::string const& value) {
    headers = curl_slist_append(headers, (name + ": " + value).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

size_t CurlHttpRequest::curlHeaderHandler(char* buffer, size_t size, size_t nitems, HttpResponse* data) {
    char* ptr = (char*) memchr(buffer, ':', size * nitems);
    if (ptr != nullptr) {
        size_t iof = ptr - buffer;
        data->headers.emplace_back(std::string(buffer, iof), std::string(&buffer[iof + 2], size * nitems - iof - 2 - 2));
    }
    return nitems * size;
}

size_t CurlHttpRequest::curlOutputHandler(void* ptr, size_t size, size_t nmemb, std::ostream* s) {
    s->write((char*) ptr, size * nmemb);
    return size * nmemb;
}

int CurlHttpRequest::curlDebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
    if (type == CURLINFO_HEADER_IN || type == CURLINFO_HEADER_OUT ||
            type == CURLINFO_DATA_IN || type == CURLINFO_DATA_OUT || type == CURLINFO_TEXT)
        write(2, data, size);
    return 0;
}

HttpResponse CurlHttpRequest::send() {
    HttpResponse response;

#ifdef CLL_DEBUG_NETWORK
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curlDebugCallback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlHeaderHandler);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

    std::stringstream body;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlOutputHandler);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

    CURLcode ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
        throw CurlError(ret);
    response.body = body.str();
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);

    return response;
}