#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>

namespace cll
{

  class Configuration;

  struct CachedConfiguration
  {
    std::chrono::system_clock::time_point expires;
    std::chrono::seconds refreshInterval;
    std::string etag;
    nlohmann::json data;
  };

  class ConfigurationCache
  {
   public:
    virtual ~ConfigurationCache() = default;
    virtual bool readFromCache(std::string const& url,
                               CachedConfiguration& config) = 0;
    virtual void writeConfigToCache(std::string const& url,
                                    CachedConfiguration const& config) = 0;
  };

}  // namespace cll