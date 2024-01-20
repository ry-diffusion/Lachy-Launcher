#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "../common/connection.h"
#include "../common/connection_handler.h"

namespace simpleipc {
namespace client {

class service_client_impl : public connection_handler {

public:
    class callback_interface {

    public:
        virtual void connection_opened() = 0;

        virtual void connection_closed() = 0;

        virtual void handle_message(response_message const& msg) = 0;

        virtual void handle_message(error_message const& msg) = 0;

    };

    virtual ~service_client_impl() = default;

    virtual void set_callback_interface(callback_interface* cb) = 0;

    virtual void open(std::string const& path) = 0;

    virtual void send_message(rpc_message const& msg) = 0;

    virtual connection* get_connection() = 0;

    virtual void close() = 0;

};

class service_client_impl_factory {

public:
    static std::unique_ptr<service_client_impl> create_platform_service();

};

}
}