#include <cll/http/http_client.h>

namespace cll {
namespace http {

class CurlHttpClient : public HttpClient {

public:
    std::unique_ptr<HttpRequest> createRequest() override;

};

}
}
