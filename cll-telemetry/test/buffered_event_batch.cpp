#include <cll/buffered_event_batch.h>
#include <gtest/gtest.h>
#include <cll/file_event_batch.h>
#include "event_batch.h"

using namespace cll;

class BufferedBatchTest : public ::testing::Test {
protected:
    BufferedEventBatch batch;

    BufferedBatchTest() : batch(std::unique_ptr<EventBatch>(new FileEventBatch("test_data")), 5) {
    }

    ~BufferedBatchTest() {
        remove("test_data");
    }
};


TEST_F(BufferedBatchTest, ReadIncremental) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncremental(batch);
}

TEST_F(BufferedBatchTest, ReadIncrementalWithRemoval) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncrementalWithRemoval(batch);
}