#include <cll/file_configuration_cache.h>

#include <fstream>
#include <log.h>

using namespace cll;

void FileConfigurationCache::load() {
    std::ifstream fs(path);
    if (!fs)
        return;
    try {
        fs >> data;
    } catch (std::exception& e) {
        Log::warn("FileConfigurationCache", "Failed to read configuration cache: %s", e.what());
    }
}

void FileConfigurationCache::store() {
    std::ofstream fs (path);
    if (!fs) {
        Log::warn("FileConfigurationCache", "Failed to open file for configuration cache");
        return;
    }
    fs << data;
}

bool FileConfigurationCache::readFromCache(std::string const& url, CachedConfiguration& config) {
    auto en = data.find(url);
    if (en == data.end())
        return false;;
    config.refreshInterval = std::chrono::seconds(en->value("refreshInterval", 0LL));
    config.expires = std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(en->value("expires", 0LL)));
    config.etag = en->value("etag", std::string());
    config.data = en->value("data", nlohmann::json::object());
    return true;
}

void FileConfigurationCache::writeConfigToCache(std::string const& url, CachedConfiguration const& config) {
    auto& en = data[url];
    en["expires"] = std::chrono::duration_cast<std::chrono::milliseconds>(config.expires.time_since_epoch()).count();
    en["refreshInterval"] = config.refreshInterval.count();
    en["etag"] = config.etag;
    en["data"] = config.data;
    store();
}