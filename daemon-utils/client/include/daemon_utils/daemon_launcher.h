#pragma once

#include <string>
#include <simpleipc/client/service_client_impl.h>

namespace daemon_utils {

class daemon_launcher {

protected:
    std::string service_path;

public:
    daemon_launcher(std::string const& service_path) : service_path(service_path) {}

    virtual ~daemon_launcher() {}

    virtual pid_t start();

    virtual std::vector<std::string> get_arguments() = 0;

    virtual std::string get_cwd() { return "/"; }

    void open(simpleipc::client::service_client_impl& impl);

};

}