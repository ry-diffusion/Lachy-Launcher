#pragma once

#include <atomic>
#include "event.h"
#include "event_uploader.h"
#include "event_serializer.h"
#include "event_serializer_extensions.h"
#include "configuration_manager.h"
#include "task_with_delay_thread.h"
#include "memory_event_batch.h"
#include "http/http_client.h"

namespace cll {

class EventManager {

private:
    std::string iKey;
    std::unique_ptr<http::HttpClient> httpClient;
    ConfigurationManager config;
    std::mutex configUpdateMutex;
    std::atomic<size_t> uploaderMaxEvents, uploaderMaxSize;
    EventUploader uploader;
    EventSerializer serializer;
    DefaultExtensions defaultSerializerExtensions;
    std::unique_ptr<EventBatch> normalStorageBatch, criticalStorageBatch;
    MemoryEventBatch realtimeMemoryBatch;
    std::unique_ptr<TaskWithDelayThread> mainUploadTask, realtimeUploadTask;

    void updateConfigIfNeeded();
    void onConfigurationUpdated();

    void uploadTasks();
    void uploadRealtimeTasks();

public:
    /**
     * Creates the Event Manager with the specified application-specific instrumentation key.
     * @param iKey the instrumentation key
     * @param batchesDir the directory where the the batches will be stored
     * @param cacheDir the directory where cached information will be stored
     */
    EventManager(std::string const& iKey, std::string const& batchesDir, std::string const& cacheDir);

    ~EventManager();

    /**
     * Gets the currently used instrumentation key, as passed in the constructor
     * @return the instrumentation key
     */
    inline std::string const& getIKey() const { return iKey; }

    /***
     * Adds an EventUploader step, which allows you to add custom authentication headers to the requests.
     * @param step the step to add
     */
    void addUploadStep(EventUploadStep& step) {
        uploader.addStep(step);
    }

    /**
     * Sets the application id and version, which will be included in the uploaded events.
     * @param appId application id in the format of 'T:name', eg. 'A:com.package.name' for an Android app
     * @param appVer the application version string
     */
    void setApp(std::string appId, std::string appVer) {
        serializer.setApp(std::move(appId), std::move(appVer));
    }

    /**
     * Starts the event upload threads as needed. No setters should be called after this is called.
     */
    void start();


    /**
     * Adds an event to upload
     * @param event the event to add
     */
    void add(Event const& event);

};

}