#include <cll/task_with_delay_thread.h>

using namespace cll;

thread_local std::condition_variable* TaskWithDelayThread::currentCondVar;
thread_local bool* TaskWithDelayThread::currentIsStopping;
thread_local std::unique_lock<std::mutex>* TaskWithDelayThread::currentLock;

void TaskWithDelayThread::doThreadLoop() {
    std::unique_lock<std::mutex> lock(mutex);
    currentCondVar = &cv;
    currentIsStopping = &stopping;
    currentLock = &lock;
    while (hasPendingTask) {
        if (!runImmediately && !stopping)
            cv.wait_for(lock, delay);
        hasPendingTask = false;
        runImmediately = false;
        lock.unlock();
        function();
        lock.lock();
    }
    running = false;
    currentCondVar = nullptr;
    currentIsStopping = nullptr;
    currentLock = nullptr;
}

TaskWithDelayThread::~TaskWithDelayThread() {
    terminate();
}

void TaskWithDelayThread::terminate() {
    std::unique_lock<std::mutex> lock(mutex);
    runImmediately = true;
    stopping = true;
    lock.unlock();
    cv.notify_all();
    if (thread.joinable())
        thread.join();
}

void TaskWithDelayThread::requestRun(bool immediate) {
    std::unique_lock<std::mutex> lock(mutex);
    hasPendingTask = true;
    if (immediate)
        runImmediately = true;
    if (!running) {
        running = true;
        if (thread.joinable())
            thread.join();
        thread = std::thread(std::bind(&TaskWithDelayThread::doThreadLoop, this));
    } else if (immediate) {
        lock.unlock();
        cv.notify_all();
    }
}