#pragma once

#include <nlohmann/json.hpp>
#include "../message/message_container.h"

namespace simpleipc {

class connection_internal;

namespace encoding {

class encoding {

public:
    virtual ~encoding() = default;

    virtual const char* name() const = 0;

    virtual void send_message(connection_internal& conn, rpc_message const& msg) = 0;

    virtual void send_message(connection_internal& conn, response_message const& msg) = 0;

    virtual void send_message(connection_internal& conn, error_message const& msg) = 0;

    // negative if not complete (-1), otherwise the byte offset of the end
    virtual ssize_t check_read_message_complete(const char* buf, size_t buf_size, size_t last_read_off) = 0;

    virtual void read_message(const char* buf, size_t buf_size, message_container& ret) = 0;

};

}

}