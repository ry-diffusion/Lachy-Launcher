#pragma once

#include <string>
#include <unordered_map>

#include "http_client.h"
#include "http_response.h"

namespace cll
{
  namespace http
  {

    class MockHttpClient : public HttpClient
    {
     private:
      std::unordered_map<std::string, HttpResponse> responses;

     public:
      void addMockedResponse(std::string const& url, HttpResponse resp)
      {
        responses[url] = std::move(resp);
      }

      HttpResponse getMockedResponse(std::string const& url);

      std::unique_ptr<HttpRequest> createRequest() override;
    };

  }  // namespace http
}  // namespace cll