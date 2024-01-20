#pragma once

#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

class ConfigurationCache;
class CachedConfiguration;
namespace http { class HttpClient; }

template <typename T>
class ConfigurationProperty {

private:
    T value;
    bool valueSet = false;

public:
    void reset() {
        value = T();
        valueSet = false;
    }

    void set(T value) {
        this->value = value;
        valueSet = true;
    }

    void set(nlohmann::json const& json, std::string const& name);

    T const& get() const {
        return value;
    }

    bool isSet() const {
        return valueSet;
    }

};

class Configuration {

protected:
    std::chrono::system_clock::time_point downloadMinRetryTime;
    int downloadAttempt = 0;

public:
    std::string const url;
    bool downloaded = false;
    std::chrono::system_clock::time_point expires;
    ConfigurationProperty<int> maxEventSizeInBytes;
    ConfigurationProperty<int> maxEventsPerPost;
    ConfigurationProperty<int> queueDrainInterval;

    explicit Configuration(std::string url) : url(std::move(url)) {}

    bool download(http::HttpClient& client, ConfigurationCache* cache);

    bool loadFromCache(ConfigurationCache* cache);

    bool needsRedownload() const {
        return !downloaded ||
                std::chrono::system_clock::now() >= expires;
    }

private:
    void applyFromJson(nlohmann::json const& json);

    void importCached(CachedConfiguration const& cached);

    static nlohmann::json safeParseJson(std::string const& str);

};

}