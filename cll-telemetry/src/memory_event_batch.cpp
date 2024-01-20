#include <cll/memory_event_batch.h>

using namespace cll;

bool MemoryEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> lock (mutex);
    if (items.size() >= limit)
        return false;
    items.push_back(rawData); // TODO: std::move
    return true;
}

bool MemoryEventBatch::hasEvents() const {
    std::lock_guard<std::mutex> lock (mutex);
    return !items.empty();
}

std::unique_ptr<BatchedEventList> MemoryEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (mutex);
    if (maxCount == 0 || maxSize == 0 || items.empty())
        return nullptr;
    std::vector<char> ret;
    size_t s = 0;
    size_t count = 0;
    for (auto const& i : items) {
        std::string itm = i.dump();
        size_t ns = s + itm.size() + 2;
        if (ns > maxSize)
            break;
        ret.resize(ns);
        memcpy(&ret[s], itm.data(), itm.size());
        ret[ns - 2] = '\r';
        ret[ns - 1] = '\n';
        s = ns;
        if (++count == maxCount)
            break;
    }
    return std::unique_ptr<BatchedEventList>(new VectorBatchedEventList(ret, count, count < items.size()));
}

void MemoryEventBatch::onEventsUploaded(BatchedEventList& events) {
    std::lock_guard<std::mutex> lock (mutex);
    items.erase(items.begin(), items.begin() + ((VectorBatchedEventList&) events).getEventCount());
}

std::vector<nlohmann::json> MemoryEventBatch::transferAllEvents() {
    std::lock_guard<std::mutex> lock (mutex);
    return std::move(items);
}