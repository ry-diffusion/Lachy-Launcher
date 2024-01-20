#pragma once

#include <string>
#include "configuration_cache.h"

namespace cll {

class FileConfigurationCache : public ConfigurationCache {

private:
    std::string path;
    nlohmann::json data = nlohmann::json::object();

    void load();
    void store();

public:
    explicit FileConfigurationCache(std::string path) : path(std::move(path)) {
        load();
    }

    bool readFromCache(std::string const& url, CachedConfiguration& config) override;
    void writeConfigToCache(std::string const& url, CachedConfiguration const& config) override;


};

}