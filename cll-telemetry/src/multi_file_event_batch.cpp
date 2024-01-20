#include <cll/multi_file_event_batch.h>
#include <dirent.h>
#include <sstream>

using namespace cll;

MultiFileEventBatch::MultiFileEventBatch(std::string path, std::string prefix, std::string suffix, size_t fileMaxEvents,
                                         size_t fileMaxSize) :
        path(std::move(path)), prefix(std::move(prefix)), suffix(std::move(suffix)), fileMaxEvents(fileMaxEvents),
        fileMaxSize(fileMaxSize) {
    if (this->path.length() > 0 && this->path[this->path.length() - 1] != '/')
        this->path = this->path + '/';
    oldBatches = getBatches();
    if (!oldBatches.empty()) {
        newestBatchId = oldBatches.back();
        oldBatches.pop_back();
    }
    newestBatch = openBatch(newestBatchId, true);
}

std::list<long long> MultiFileEventBatch::getBatches() {
    DIR* dir = opendir(path.c_str());
    std::list<long long> ret;
    if (dir == nullptr)
        return ret;
    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_REG) {
            size_t len = strlen(dp->d_name);
            if (len < prefix.length() + suffix.length() ||
                    memcmp(dp->d_name, prefix.data(), prefix.length()) != 0 ||
                    memcmp(&dp->d_name[len - suffix.length()], suffix.data(), suffix.length()) != 0)
                continue;
            std::string e(&dp->d_name[prefix.length()], len - prefix.length() - suffix.length());
            try {
                ret.emplace_back(std::stoull(e));
            } catch (std::exception& e) {
            }
        }
    }
    closedir(dir);
    ret.sort();
    return ret;
}

std::string MultiFileEventBatch::getBatchFileName(long long id) {
    std::stringstream ss;
    ss << path << prefix << id << suffix;
    return ss.str();
}

std::unique_ptr<FileEventBatch> MultiFileEventBatch::openBatch(long long id, bool write) {
    std::unique_ptr<FileEventBatch> ev(new FileEventBatch(getBatchFileName(id)));
    ev->setLimit(fileMaxEvents, fileMaxSize);
    if (!write)
        ev->setFinalized();
    return ev;
}

void MultiFileEventBatch::checkOldestBatch() {
    if (!oldestBatch) {
        while (!oldBatches.empty()) {
            long long fr = oldBatches.front();
            oldBatches.pop_front();
            oldestBatch = openBatch(fr);
            if (oldestBatch->hasEvents()) {
                // we found an old batch with events, good
                break;
            } else {
                // delete it
                std::string path = oldestBatch->getPath();
                oldestBatch.reset();
                remove(path.c_str());
            }
        }
    }
}

bool MultiFileEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    std::string data = rawData.dump() + "\r\n";
    if (!newestBatch->canAddEvent(data.size())) {
        newestBatch->setFinalized();
        if (!oldestBatch && oldBatches.empty()) {
            oldestBatch = std::move(newestBatch);
        } else {
            oldBatches.push_back(newestBatchId);
        }
        newestBatchId++;
        newestBatch = openBatch(newestBatchId, true);
    }
    return newestBatch->addEvent(data);
}

std::unique_ptr<BatchedEventList> MultiFileEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    checkOldestBatch();
    if (oldestBatch) {
        std::unique_ptr<BatchedEventList> ret = oldestBatch->getEventsForUpload(maxCount, maxSize);
        if (!ret)
            return nullptr;
        return std::unique_ptr<BatchedEventList>(new EventList(std::move(ret), true));
    } else {
        // the newest batch is not set as the oldest batch, so we need to check for it here
        std::unique_ptr<BatchedEventList> ret = newestBatch->getEventsForUpload(maxCount, maxSize);
        if (!ret)
            return nullptr;
        return std::unique_ptr<BatchedEventList>(new EventList(std::move(ret), false));
    }
}

void MultiFileEventBatch::onEventsUploaded(BatchedEventList& events) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    if (oldestBatch) {
        oldestBatch->onEventsUploaded(*((EventList&) events).wrapped);
        // if the batch has no more events, reset the pointer; the oldestBatch pointer will be auto recreated as needed
        // when checkOldestBatch() is called
        // the batch file will be automatically deleted by the onEventsUploaded if there are no more events
        if (!oldestBatch->hasEvents()) {
            oldestBatch.reset();
        }
    } else {
        newestBatch->onEventsUploaded(*((EventList&) events).wrapped);
    }
}

bool MultiFileEventBatch::hasEvents() const {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    return !oldBatches.empty() || oldestBatch != nullptr || newestBatch->hasEvents();
}

void MultiFileEventBatch::setFileLimits(size_t maxEvents, size_t maxSize) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    fileMaxSize = maxSize;
    fileMaxEvents = maxEvents;
    if (oldestBatch)
        oldestBatch->setLimit(maxEvents, maxSize);
    newestBatch->setLimit(maxEvents, maxSize);
}