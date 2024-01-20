#pragma once

#include <stdexcept>
#include <nlohmann/json.hpp>
#include "rpc_call_exception.h"

namespace simpleipc {

class rpc_message;
class response_message;
class error_message;

class connection_handler {

public:
    virtual void connection_closed(connection& connection) {}

    virtual void handle_message_parse_error(connection& connection, std::exception_ptr ex_ptr,
                                            rpc_call_exception_interface const& ex_as_rpc) {
        std::rethrow_exception(ex_ptr);
    }

    virtual void handle_message(connection& connection, rpc_message const& msg) {
        throw std::runtime_error("Unsupported message");
    }

    virtual void handle_message(connection& connection, response_message const& msg) {
        throw std::runtime_error("Unsupported message");
    }

    virtual void handle_message(connection& connection, error_message const& msg) {
        throw std::runtime_error("Unsupported message");
    }

};

}