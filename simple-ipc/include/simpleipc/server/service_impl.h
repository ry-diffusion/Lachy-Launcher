#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "../common/connection.h"
#include "../common/connection_handler.h"

namespace simpleipc {
namespace server {

class service_impl : public connection_handler {

public:
    class callback_interface {

    public:
        virtual void handle_message(connection& client, rpc_message const& req) = 0;

        virtual void on_client_connected(connection& client) = 0;

        virtual void on_client_disconnected(connection& client) = 0;

    };

    virtual ~service_impl() = default;

    virtual void set_callback_interface(callback_interface* cb) = 0;

    virtual void bind(std::string const& path) = 0;

    virtual void close() = 0;

};

class service_impl_factory {

public:
    static std::unique_ptr<service_impl> create_platform_service();

};

}
}