#pragma once

#include <simpleipc/common/message/rpc_message.h>
#include <simpleipc/common/message/response_message.h>
#include <simpleipc/common/message/error_message.h>

namespace simpleipc {

struct message_container {
    enum class message_type {
        none, rpc, response, error
    };

    message_type type = message_type::none;
    rpc_message message_rpc;
    response_message message_response;
    error_message message_error;

    void set(rpc_message message_rpc) {
        type = message_type::rpc;
        this->message_rpc = std::move(message_rpc);
    }
    void set(response_message message_response) {
        type = message_type::response;
        this->message_response = std::move(message_response);
    }
    void set(error_message message_error) {
        type = message_type::error;
        this->message_error = std::move(message_error);
    }

    void clear() {
        switch (type) {
            case message_type::rpc:
                message_rpc = rpc_message();
                break;
            case message_type::response:
                message_response = response_message();
                break;
            case message_type::error:
                message_error = error_message();
                break;
            default:
                break;
        }
        type = message_type::none;
    }
};

}