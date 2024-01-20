#include <cll/event_serializer.h>
#include <cll/event.h>
#include <random>
#include <sys/utsname.h>

using namespace cll;

EventSerializer::EventSerializer() {
    std::random_device dev;
    std::uniform_int_distribution<long long> ldist(0, std::numeric_limits<long long>::max());
    epoch = ldist(dev);

    struct utsname osInfo;
    if (uname(&osInfo) == 0) {
        os = osInfo.sysname;
        osVer = osInfo.release;
    }
}

std::string EventSerializer::getEventTimeAsString(std::chrono::system_clock::time_point timepoint) {
    using namespace std::chrono;
    char timestamp[32];
    time_t time = system_clock::to_time_t(timepoint);
    strftime(timestamp, sizeof(timestamp), "%FT%T", gmtime(&time));
    auto timestampLen = strlen(timestamp);
    auto timeMs = duration_cast<milliseconds>(timepoint.time_since_epoch()) -
            duration_cast<milliseconds>(duration_cast<seconds>(timepoint.time_since_epoch()));
    snprintf(&timestamp[timestampLen], sizeof(timestamp) - timestampLen, ".%03dZ", (int) timeMs.count());
    return std::string(timestamp);
}

nlohmann::json EventSerializer::createEnvelopeFor(Event const& ev) {
    nlohmann::json envelope;
    envelope["ver"] = "2.1";
    envelope["name"] = ev.getName();
    envelope["data"] = ev.getData();
    envelope["time"] = getEventTimeAsString(ev.getTime());
    envelope["popSample"] = 100.0; // TODO: Implement sample rates
    envelope["epoch"] = std::to_string(epoch);
    envelope["seqNum"] = seqNum++;
    envelope["iKey"] = iKey;
    envelope["flags"] = ev.getFlags();
    if (!os.empty())
        envelope["os"] = os;
    if (!osVer.empty())
        envelope["osVer"] = osVer;
    if (!appId.empty())
        envelope["appId"] = appId;
    if (!appVer.empty())
        envelope["appVer"] = appVer;
    if (!extensions.empty()) {
        auto& ext = envelope["ext"] = nlohmann::json::object();
        for (auto const& e : extensions)
            ext[e->getName()] = e->build(ev);
    }
    return envelope;
}