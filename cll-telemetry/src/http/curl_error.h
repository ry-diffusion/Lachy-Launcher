#pragma once

#include <string>
#include <curl/curl.h>
#include <cll/http/http_error.h>

namespace cll {
namespace http {

class CurlError : public HttpError {

private:
    CURLcode _code;

public:
    CurlError(CURLcode code) : _code(code) {
    }

    inline CURLcode code() const {
        return _code;
    }

    const char* what() const noexcept override {
        return curl_easy_strerror(code());
    }

};

}
}