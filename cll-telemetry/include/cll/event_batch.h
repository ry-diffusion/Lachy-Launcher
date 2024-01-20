#pragma once

#include "event.h"

namespace cll {

class BatchedEventList {

public:
    virtual ~BatchedEventList() = default;
    virtual const char* getData() const = 0;
    virtual size_t getDataSize() const = 0;
    virtual size_t getEventCount() const = 0;
    virtual bool hasMoreEvents() const = 0;

};

class EventBatch {

public:
    virtual ~EventBatch() = default;

    virtual bool addEvent(nlohmann::json const& rawData) = 0;

    virtual std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) = 0;

    virtual void onEventsUploaded(BatchedEventList& events) = 0;

    virtual bool hasEvents() const = 0;

};


class VectorBatchedEventList : public BatchedEventList {

private:
    std::vector<char> data;
    size_t events;
    bool moreEvents;

public:
    explicit VectorBatchedEventList(std::vector<char> data, size_t events, bool hasMoreEvents) :
            data(std::move(data)), events(events), moreEvents(hasMoreEvents) {}

    const char* getData() const override {
        return data.data();
    }

    size_t getDataSize() const override {
        return data.size();
    }

    size_t getEventCount() const override {
        return events;
    }

    bool hasMoreEvents() const override {
        return moreEvents;
    }

};

}