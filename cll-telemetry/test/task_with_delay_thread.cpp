#include <cll/task_with_delay_thread.h>
#include <gtest/gtest.h>
#include <future>

using namespace cll;

TEST(TaskWithDelayThreadTest, Grouping) {
    std::promise<bool> promise;
    bool alreadyCalled = false;
    auto future = promise.get_future();
    TaskWithDelayThread thread (std::chrono::milliseconds(5), [&promise, &alreadyCalled]() {
        ASSERT_FALSE(alreadyCalled);
        alreadyCalled = true;
        promise.set_value(true);
    });
    auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; i++)
        thread.requestRun();
    EXPECT_TRUE(future.get());
    auto endTime = std::chrono::steady_clock::now();
    ASSERT_GE(endTime - startTime, std::chrono::milliseconds(5));
}

TEST(TaskWithDelayThreadTest, ImmediateTask) {
    std::promise<bool> promise;
    auto future = promise.get_future();
    TaskWithDelayThread thread (std::chrono::seconds(1), [&promise]() {
        promise.set_value(true);
    });
    thread.requestRun(true);
    auto startTime = std::chrono::steady_clock::now();
    EXPECT_TRUE(future.get());
    auto endTime = std::chrono::steady_clock::now();
    ASSERT_LE(endTime - startTime, std::chrono::milliseconds(10));
}

TEST(TaskWithDelayThreadTest, ImmediateDestroy) {
    std::promise<bool> promise;
    auto future = promise.get_future();
    auto startTime = std::chrono::steady_clock::now();
    {
        TaskWithDelayThread thread (std::chrono::seconds(1), [&promise]() {
            promise.set_value(true);
        });
        thread.requestRun();
    }
    EXPECT_TRUE(future.get());
    auto endTime = std::chrono::steady_clock::now();
    ASSERT_LE(endTime - startTime, std::chrono::milliseconds(10));
}