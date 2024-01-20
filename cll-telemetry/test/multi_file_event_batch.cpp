#include <cll/multi_file_event_batch.h>
#include <gtest/gtest.h>
#include <dirent.h>
#include "event_batch.h"

using namespace cll;

static void CleanUpBatch(MultiFileEventBatch& batch) {
    DIR* dir = opendir(batch.getPath().c_str());
    if (dir != nullptr) {
        struct dirent* dp;
        while ((dp = readdir(dir)) != nullptr) {
            if (dp->d_type == DT_REG)
                remove((batch.getPath() + dp->d_name).c_str());
        }
        closedir(dir);
    }
    remove(batch.getPath().c_str());
}

class MultiFileEventBatchTest : public ::testing::Test {
public:
    std::unique_ptr<MultiFileEventBatch> batch;

    ~MultiFileEventBatchTest() {
        CleanUpBatch(*batch);
    }
};

TEST_F(MultiFileEventBatchTest, BasicTest) {
    mkdir("multifile_test", 0700);
    batch = std::unique_ptr<MultiFileEventBatch>(new MultiFileEventBatch("multifile_test", "events", ".txt", 2, 1024));
    EventBatchTest::BasicTest(*batch);
}

TEST_F(MultiFileEventBatchTest, ReadIncrementalWithRemoval) {
    mkdir("multifile_test", 0700);
    batch = std::unique_ptr<MultiFileEventBatch>(new MultiFileEventBatch("multifile_test", "events", ".txt", 10, 320));
    EventBatchTest::SetUpTestEvents(*batch);
    EventBatchTest::ReadIncrementalWithRemoval(*batch);
}