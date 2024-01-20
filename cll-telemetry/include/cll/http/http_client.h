#pragma once

#include <memory>
#include "http_request.h"

namespace cll {
namespace http {

class HttpClient {

public:
    static std::unique_ptr<HttpClient> createPlatformClient();


    virtual ~HttpClient() {}

    virtual std::unique_ptr<HttpRequest> createRequest() = 0;

};

}
}