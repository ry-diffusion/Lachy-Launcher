#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <condition_variable>

namespace cll {

/**
 * This is a simple class that starts a thread to run the specified task with a delay.
 * The class waits before starting the task for a specified time amount, as specified by the delay constructor parameter.
 */
class TaskWithDelayThread {

private:
    std::chrono::nanoseconds delay;
    std::function<void ()> function;
    std::thread thread;
    mutable std::mutex mutex;
    std::condition_variable cv;
    bool running = false, hasPendingTask = false, runImmediately = false;
    bool stopping = false;

    static thread_local std::condition_variable* currentCondVar;
    static thread_local bool* currentIsStopping;
    static thread_local std::unique_lock<std::mutex>* currentLock;

    void doThreadLoop();

public:
    /**
     * Sleeps for the specified amount of time. This function may return before the requested time if the thread is
     * stopping.
     * @param time the time to sleep for
     */
    template <typename T>
    static void sleep(T time) {
        currentLock->lock();
        if (!currentIsStopping)
            currentCondVar->wait_for(*currentLock, time);
        currentLock->unlock();
    }

    template <typename Rep, typename Period>
    explicit TaskWithDelayThread(std::chrono::duration<Rep, Period> delay, std::function<void ()> function) :
            function(std::move(function)) {
        this->delay = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
    }

    ~TaskWithDelayThread();

    /**
     * Checks whether either the terminate() function or the destructor has been called.
     * @return whether the thread is stopping
     */
    bool isStopping() const {
        std::lock_guard<std::mutex> lock(mutex);
        return stopping;
    }

    /**
     * Request a run of the provided function
     * @param immediate whether the run should skip the provided delay
     */
    void requestRun(bool immediate = false);

    /**
     * Stops the thread in a safe but non-reversible way. The function blocks until the thread is stopped.
     * No more runs should be requested after this function is called.
     */
    void terminate();

};

}