#pragma once

#include <exception>

namespace cll
{
  namespace http
  {

    class HttpError : public std::exception
    {
     public:
      HttpError()
      {
      }
    };

  }  // namespace http
}  // namespace cll