#include <cll/event_manager.h>
#include <log.h>
#include <cll/file_configuration_cache.h>
#include <cll/multi_file_event_batch.h>
#include <cll/buffered_event_batch.h>

using namespace cll;

EventManager::EventManager(std::string const& iKey, std::string const& batchesDir, std::string const& cacheDir)
        : iKey(iKey), httpClient(http::HttpClient::createPlatformClient()), uploader(*httpClient) {
    config.setCache(std::unique_ptr<ConfigurationCache>(new FileConfigurationCache(cacheDir + "/config_cache.json")));
    config.addDefaultConfigurations(iKey);
    config.addUpdateCallback(std::bind(&EventManager::onConfigurationUpdated, this));
    config.loadCachedConfigs();

    serializer.setIKey(iKey);
    defaultSerializerExtensions.addTo(serializer);

    uploaderMaxSize.store((size_t) config.getMaxEventSizeInBytes());
    uploaderMaxEvents.store((size_t) config.getMaxEventsPerPost());

    normalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "normal", ".txt", (size_t) config.getMaxEventsPerPost(),
            (size_t) config.getMaxEventSizeInBytes()));
    normalStorageBatch = std::unique_ptr<EventBatch>(new BufferedEventBatch(std::move(normalStorageBatch), 50));

    criticalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "crit", ".txt", (size_t) config.getMaxEventsPerPost(),
            (size_t) config.getMaxEventSizeInBytes()));

    mainUploadTask = std::unique_ptr<TaskWithDelayThread>(new TaskWithDelayThread(
            std::chrono::minutes(config.getQueueDrainInterval()),
            std::bind(&EventManager::uploadTasks, this)));
    realtimeUploadTask = std::unique_ptr<TaskWithDelayThread>(new TaskWithDelayThread(
            std::chrono::milliseconds(50),
            std::bind(&EventManager::uploadRealtimeTasks, this)));

    updateConfigIfNeeded();
}

EventManager::~EventManager() {
    mainUploadTask->terminate();
    realtimeUploadTask->terminate();
}

void EventManager::start() {
    mainUploadTask->requestRun(true);
}

void EventManager::updateConfigIfNeeded() {
    std::lock_guard<std::mutex> lock (configUpdateMutex);
    config.downloadConfigs(*httpClient);
}

void EventManager::onConfigurationUpdated() {
    ((MultiFileEventBatch&) *((BufferedEventBatch&) *normalStorageBatch).getWrapped()).setFileLimits(
            (size_t) config.getMaxEventsPerPost(), (size_t) config.getMaxEventSizeInBytes());
    ((MultiFileEventBatch&) *criticalStorageBatch).setFileLimits(
            (size_t) config.getMaxEventsPerPost(), (size_t) config.getMaxEventSizeInBytes());
    uploaderMaxSize.store((size_t) config.getMaxEventSizeInBytes());
    uploaderMaxEvents.store((size_t) config.getMaxEventsPerPost());
}

void EventManager::uploadTasks() {
    const std::chrono::milliseconds baseBackoffTime (std::chrono::seconds(5));
    std::chrono::milliseconds nextBackoffTime = baseBackoffTime;
    updateConfigIfNeeded();
    while (!mainUploadTask->isStopping() && (normalStorageBatch->hasEvents() || criticalStorageBatch->hasEvents())) {
        updateConfigIfNeeded();
        EventUploadStatus status;
        if (criticalStorageBatch->hasEvents())
            status = uploader.sendEvents(*criticalStorageBatch, uploaderMaxEvents, uploaderMaxSize);
        else if (normalStorageBatch->hasEvents())
            status = uploader.sendEvents(*normalStorageBatch, uploaderMaxEvents, uploaderMaxSize);
        else
            break;
        if (status || status.state == EventUploadStatus::State::ErrorRateLimit)
            nextBackoffTime = baseBackoffTime;
        if (status)
            continue;
        if (status.state == EventUploadStatus::State::ErrorRateLimit) {
            TaskWithDelayThread::sleep(status.retryAfter);
        } else {
            TaskWithDelayThread::sleep(nextBackoffTime);
            nextBackoffTime *= 2;
        }
    }
}

void EventManager::uploadRealtimeTasks() {
    while (realtimeMemoryBatch.hasEvents()) {
        updateConfigIfNeeded();
        if (!uploader.sendEvents(realtimeMemoryBatch, uploaderMaxEvents, uploaderMaxSize)) {
            // if the upload fails, transfer the events to the disk-backed queues, and queue an immediate reupload
            // this lets us handle the backoff only in that function
            for (auto const& ev : realtimeMemoryBatch.transferAllEvents()) {
                EventFlags flags = (EventFlags) ev.value("flags", 0);
                if (EventFlagSet(flags, EventFlags::PersistenceCritical))
                    criticalStorageBatch->addEvent(ev);
                else
                    normalStorageBatch->addEvent(ev);
            }
            mainUploadTask->requestRun(true);
        }
    }
}

void EventManager::add(Event const& event) {
    auto serialized = serializer.createEnvelopeFor(event);
    bool realtime = false;
    if (EventFlagSet(event.getFlags(), EventFlags::LatencyRealtime)) {
        if (realtimeMemoryBatch.addEvent(serialized)) {
            realtimeUploadTask->requestRun(false);
            return;
        }
        realtime = true;
    }
    bool added;
    if (EventFlagSet(event.getFlags(), EventFlags::PersistenceCritical))
        added = criticalStorageBatch->addEvent(serialized);
    else
        added = normalStorageBatch->addEvent(serialized);

    if (added)
        mainUploadTask->requestRun(realtime);
    else
        Log::warn("EventManager", "Event dropped");
}