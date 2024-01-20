#include <cll/memory_event_batch.h>
#include <gtest/gtest.h>
#include "event_batch.h"

using namespace cll;

TEST(MemoryEventBatchTest, BasicTest) {
    MemoryEventBatch batch;
    EventBatchTest::BasicTest(batch);
}

TEST(MemoryEventBatchTest, OverflowTest) {
    MemoryEventBatch batch (1);
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_FALSE(batch.addEvent(event));
}

TEST(MemoryEventBatchTest, ReadIncremental) {
    MemoryEventBatch batch (DEFAULT_TEST_EVENT_COUNT);
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncremental(batch);
}

TEST(MemoryEventBatchTest, ReadIncrementalWithRemoval) {
    MemoryEventBatch batch (DEFAULT_TEST_EVENT_COUNT);
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncrementalWithRemoval(batch);
}