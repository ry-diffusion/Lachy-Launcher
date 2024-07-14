#pragma once

#include <cll/http/http_error.h>
#include <curl/curl.h>

#include <string>

namespace cll
{
  namespace http
  {

    class CurlError : public HttpError
    {
     private:
      CURLcode _code;

     public:
      CurlError(CURLcode code) : _code(code)
      {
      }

      inline CURLcode code() const
      {
        return _code;
      }

      const char* what() const noexcept override
      {
        return curl_easy_strerror(code());
      }
    };

  }  // namespace http
}  // namespace cll