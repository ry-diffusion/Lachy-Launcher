#include <cll/event_uploader.h>
#include <log.h>
#include <cll/http/http_error.h>
#include <cll/http/http_client.h>
#include <cll/event_serializer.h>
#include <cll/event_compressor.h>

using namespace cll;
using namespace cll::http;

EventUploadStatus EventUploader::sendEvents(BatchedEventList& batch, bool compress, bool canRetry) {
    auto req = client.createRequest();
    req->setUrl(url);
    req->setMethod(HttpMethod::POST);
    req->addHeader("Content-Type", "application/x-json-stream; charset=utf-8");
    req->addHeader("X-UploadTime", getUploadTimestamp());
    std::string compressed;
    if (compress) {
        compressed = EventCompressor::compress(batch.getData(), batch.getDataSize());
        req->setPostData(compressed.data(), compressed.size());
        req->addHeader("Accept", "application/json");
        req->addHeader("Accept-Encoding", "gzip, deflate");
        req->addHeader("Content-Encoding", "deflate");
    } else {
        req->setPostData(batch.getData(), batch.getDataSize());
    }

    EventUploadRequest userReq (batch);
    for (auto const& h : steps)
        h->onRequest(userReq);
    for (auto const& h : userReq.headers)
        req->addHeader(h.first, h.second);

    HttpResponse res;
    try {
        res = req->send();
    } catch (HttpError& err) {
        return EventUploadStatus::error();
    }

    if (res.status == 200)
        return EventUploadStatus::success();

    if (res.status == 429 || res.status == 503) {
        auto retryAfter = res.findHeader("Retry-After");
        if (retryAfter.empty())
            return EventUploadStatus::error();
        return EventUploadStatus::rateLimit(std::chrono::seconds(std::stoull(retryAfter)));
    }
    if (res.status == 401) {
        bool retry = false;
        for (auto const& h : steps)
            retry |= h->onAuthenticationFailed();
        if (retry && canRetry)
            return sendEvents(batch, compress, false);
    }
    return EventUploadStatus::error();
}

std::string EventUploader::getUploadTimestamp(std::chrono::system_clock::time_point time) {
    using namespace std::chrono;
    char timestamp[64];
    time_t timec = system_clock::to_time_t(time);
    strftime(timestamp, sizeof(timestamp), "%FT%T", gmtime(&timec));
    size_t timestampLen = strlen(timestamp);
    auto timeMs = duration_cast<microseconds>(time.time_since_epoch()) -
            duration_cast<microseconds>(duration_cast<seconds>(time.time_since_epoch()));
    snprintf(&timestamp[timestampLen], sizeof(timestamp) - timestampLen, ".%06dZ", (int) timeMs.count());
    return std::string(timestamp);
}

EventUploadStatus EventUploader::sendEvents(EventBatch& batch, size_t maxCount, size_t maxSize) {
    auto events = batch.getEventsForUpload(maxCount, maxSize);
    if (!events) {
        if (batch.hasEvents()) {
            Log::warn("EventUploader", "Failed to get any event from the batch -"
                    "dropping the first event and trying again");
            auto ev = batch.getEventsForUpload(1, SIZE_MAX);
            if (!ev)
                return EventUploadStatus::error();
            batch.onEventsUploaded(*ev);
            return sendEvents(batch, maxCount, maxSize);
        }
        return EventUploadStatus::success();
    }
    auto ret = sendEvents(*events, events->getEventCount() > 1);
    if (ret)
        batch.onEventsUploaded(*events);
    return ret;
}