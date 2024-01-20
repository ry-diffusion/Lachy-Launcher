#include <daemon_utils/auto_shutdown_service.h>

using namespace daemon_utils;

void auto_shutdown_service::run() {
    std::unique_lock<std::mutex> lock (shutdown_mutex);
    shutdown_cv.wait(lock, [this]() { return requested_shutdown; });
}

void auto_shutdown_service::request_stop() {
    shutdown_mutex.lock();
    requested_shutdown = true;
    shutdown_mutex.unlock();
    shutdown_cv.notify_all();
}