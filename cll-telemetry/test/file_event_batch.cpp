#include <gtest/gtest.h>
#include <cll/file_event_batch.h>
#include "event_batch.h"

using namespace cll;

namespace {

class FileEventBatchTest : public ::testing::Test {
protected:
    FileEventBatch batch;

    FileEventBatchTest() : batch("test_data") {
    }

    ~FileEventBatchTest() {
        remove(batch.getPath().c_str());
    }
};

TEST_F(FileEventBatchTest, BasicTest) {
    EXPECT_EQ(batch.getEventCount(), 0);
    EventBatchTest::BasicTest(batch);
    EXPECT_EQ(batch.getEventCount(), 0); // the basic test deletes the item
}

TEST_F(FileEventBatchTest, ItemCountTest) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    for (int i = 0; i < 32; i++)
        batch.addEvent(event);
    auto evs = batch.getEventsForUpload(8, 128 * 8);
    ASSERT_NE(evs, nullptr);
    batch.onEventsUploaded(*evs);
    ASSERT_EQ(batch.getEventCount(), 32 - 8);
    evs = batch.getEventsForUpload(32, 128 * 32);
    ASSERT_NE(evs, nullptr);
    batch.onEventsUploaded(*evs);
    ASSERT_EQ(batch.getEventCount(), 0);
}

TEST_F(FileEventBatchTest, CountLimitTest) {
    nlohmann::json event = {{"test", "This is a test log entry"}};

    batch.setLimit(2, 2048);

    for (int i = 0; i < 2; i++)
        ASSERT_TRUE(batch.addEvent(event));

    ASSERT_FALSE(batch.addEvent(event));

    auto evs = batch.getEventsForUpload(1, 128 * 8);
    ASSERT_NE(evs, nullptr);
    batch.onEventsUploaded(*evs);

    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_FALSE(batch.addEvent(event));
}

TEST_F(FileEventBatchTest, SizeLimitTest) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    size_t eventSize = event.dump().size();

    batch.setLimit(3, (eventSize + 2) * 2);

    for (int i = 0; i < 2; i++)
        ASSERT_TRUE(batch.addEvent(event));

    ASSERT_FALSE(batch.addEvent(event));

    auto evs = batch.getEventsForUpload(1, 128 * 8);
    ASSERT_NE(evs, nullptr);
    batch.onEventsUploaded(*evs);

    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_FALSE(batch.addEvent(event));
}

TEST(FileEventBatchCustomTest, PersistenceTest) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    auto eventStr = event.dump();
    {
        FileEventBatch batch("test_data");
        ASSERT_TRUE(batch.addEvent(event));
    }
    {
        FileEventBatch batch("test_data");
        ASSERT_EQ(batch.getEventCount(), 1);
        auto upEv = EventBatchTest::GetMessagesInEventList(batch.getEventsForUpload(10, 512).get());
        ASSERT_EQ(upEv.size(), 1);
        ASSERT_EQ(upEv[0], eventStr);
    }
    remove("test_data");
}

TEST_F(FileEventBatchTest, NoAddingEventsToFinalized) {
    batch.setFinalized();
    ASSERT_FALSE(batch.addEvent(nlohmann::json::object()));
}

TEST_F(FileEventBatchTest, FinalizedDeletion) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_TRUE(batch.addEvent(event));
    batch.setFinalized();
    auto upEv = batch.getEventsForUpload(1, 128);
    ASSERT_NE(upEv, nullptr);
    batch.onEventsUploaded(*upEv);
    // Make sure the file no longer exists (it was finalized, and all events should be removed)
    ASSERT_FALSE(access(batch.getPath().c_str(), F_OK) == 0);
}

TEST_F(FileEventBatchTest, ReadIncremental) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncremental(batch);
}
TEST_F(FileEventBatchTest, ReadIncrementalWithRemoval) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncrementalWithRemoval(batch);
}

}