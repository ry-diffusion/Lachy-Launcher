#pragma once

#include <memory>
#include <vector>
#include "event.h"
#include "event_upload_step.h"
#include "event_batch.h"

namespace cll {

namespace http { class HttpClient; }

struct EventUploadStatus {

    enum class State {
        Success,
        ErrorGeneric, ErrorRateLimit
    };

    State state;
    std::chrono::seconds retryAfter;

    static EventUploadStatus success() {
        return {State::Success};
    }
    static EventUploadStatus error() {
        return {State::ErrorGeneric};
    }
    static EventUploadStatus rateLimit(std::chrono::seconds retryAfter) {
        return {State::ErrorRateLimit, retryAfter};
    }


    operator bool() const {
        return state == State::Success;
    }

};

class EventUploader {

private:
    http::HttpClient& client;
    std::string url;
    std::vector<EventUploadStep*> steps;

    static std::string getUploadTimestamp(
            std::chrono::system_clock::time_point time = std::chrono::system_clock::now());

public:
    EventUploader(http::HttpClient& client, std::string url = "https://vortex.data.microsoft.com/collect/v1") :
            client(client), url(std::move(url)) {}

    EventUploadStatus sendEvents(BatchedEventList& batch, bool compress = false, bool canRetry = true);

    EventUploadStatus sendEvents(EventBatch& batch, size_t maxCount, size_t maxSize);

    void addStep(EventUploadStep& step) {
        steps.push_back(&step);
    }

};

}