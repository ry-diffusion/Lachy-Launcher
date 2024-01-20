#pragma once

#include "event_batch.h"
#include <nlohmann/json.hpp>
#include <mutex>

namespace cll {

class MemoryEventBatch : public EventBatch {

private:
    mutable std::mutex mutex;
    std::vector<nlohmann::json> items;
    const size_t limit;

public:
    MemoryEventBatch(size_t limit = 50) : limit(limit) {}

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override;

    std::vector<nlohmann::json> transferAllEvents();

};

}