#pragma once

#include <simpleipc/server/service.h>
#include <mutex>
#include <condition_variable>

namespace daemon_utils {

enum class shutdown_policy {
    no_connections,
    never
};

class auto_shutdown_service : public simpleipc::server::service {

private:
    shutdown_policy shutdown_mode;
    int connections = 0;
    bool requested_shutdown;
    std::mutex shutdown_mutex;
    std::condition_variable shutdown_cv;

protected:
    virtual void request_stop();

public:
    auto_shutdown_service(std::unique_ptr<simpleipc::server::service_impl> impl, shutdown_policy policy)
            : service(std::move(impl)), shutdown_mode(policy) {}

    auto_shutdown_service(std::string const& path, shutdown_policy policy) : service(path), shutdown_mode(policy) {}


    void run();


    void on_client_connected(simpleipc::connection& client) override {
        connections++;
    }

    void on_client_disconnected(simpleipc::connection& client) override {
        if (--connections == 0 && shutdown_mode == shutdown_policy::no_connections) {
            request_stop();
        }
    }

};

}