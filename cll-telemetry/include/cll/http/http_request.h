#pragma once

#include <string>
#include <functional>
#include "http_response.h"

namespace cll {
namespace http {

enum class HttpMethod {
    GET, POST
};

class HttpRequest {

public:
    virtual ~HttpRequest() {}

    virtual void setUrl(std::string const& url) = 0;

    virtual void setMethod(HttpMethod method) = 0;

    virtual void setPostData(const char* data, size_t size) = 0;

    virtual void addHeader(std::string const& name, std::string const& value) = 0;

    virtual HttpResponse send() = 0;

};

}
}