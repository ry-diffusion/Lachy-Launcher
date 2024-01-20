#pragma once

#include <cll/event_batch.h>

#define DEFAULT_TEST_EVENT_COUNT 128

class EventBatchTest {

public:
    static void BasicTest(cll::EventBatch& batch);

    static void SetUpTestEvents(cll::EventBatch& batch, size_t eventCount = DEFAULT_TEST_EVENT_COUNT);

    static void ReadIncremental(cll::EventBatch& batch, size_t eventCount = DEFAULT_TEST_EVENT_COUNT);

    static void ReadIncrementalWithRemoval(cll::EventBatch& batch, size_t eventCount = DEFAULT_TEST_EVENT_COUNT);


    static nlohmann::json GetJsonFor(int eventIndex) {
        return {{"test", "This is a test log entry #" + std::to_string(eventIndex)}};
    }

    static std::vector<std::string> GetMessagesInEventList(cll::BatchedEventList* val);

};