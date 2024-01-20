#pragma once

#include <memory>
#include <list>
#include "file_event_batch.h"

namespace cll {

class MultiFileEventBatch : public EventBatch {

private:
    struct EventList : public BatchedEventList {

        std::unique_ptr<BatchedEventList> wrapped;
        bool hasMoreFiles;

        EventList(std::unique_ptr<BatchedEventList> wrapped, bool hasMore) :
                wrapped(std::move(wrapped)), hasMoreFiles(hasMore) {}

        const char* getData() const override { return wrapped->getData(); }
        size_t getDataSize() const override { return wrapped->getDataSize(); }
        size_t getEventCount() const override { return wrapped->getEventCount(); }
        bool hasMoreEvents() const override { return hasMoreFiles | wrapped->hasMoreEvents(); }

    };

    std::string path;
    std::string prefix;
    std::string suffix;
    size_t fileMaxSize;
    size_t fileMaxEvents;
    mutable std::mutex batchPointerMutex;
    std::unique_ptr<FileEventBatch> oldestBatch;
    std::list<long long> oldBatches;
    std::unique_ptr<FileEventBatch> newestBatch;
    long long newestBatchId = 0;

    std::list<long long> getBatches();

    std::string getBatchFileName(long long id);
    std::unique_ptr<FileEventBatch> openBatch(long long id, bool write = false);

    void checkOldestBatch();

public:
    MultiFileEventBatch(std::string path, std::string prefix, std::string suffix,
                        size_t fileMaxEvents, size_t fileMaxSize);

    void setFileLimits(size_t maxSize, size_t maxEvents);

    std::string const& getPath() const { return path; }

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override;

};

}