#pragma once

#include "../common/unix_connection.h"
#include "../../client/base_service_client_impl.h"

namespace simpleipc {
namespace client {

class unix_service_client_impl : public base_service_client_impl {

private:
    int fd = -1;
    std::string path;
    std::shared_ptr<unix_connection> connection;

public:
    void open(std::string const& path) override;

    void send_message(rpc_message const& msg) override;

    void close() override;

    simpleipc::connection* get_connection() override {
        return connection.get();
    }

};

}
}