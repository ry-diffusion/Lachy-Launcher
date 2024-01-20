#pragma once

#include "encoding.h"

namespace simpleipc {
namespace encoding {

class json : public encoding {

public:
    const char* name() const override { return "json"; }


    void send_message(connection_internal& conn, rpc_message const& msg) override;

    void send_message(connection_internal& conn, response_message const& msg) override;

    void send_message(connection_internal& conn, error_message const& msg) override;

    void send_json(connection_internal& conn, nlohmann::json const& data);


    ssize_t check_read_message_complete(const char* buf, size_t buf_size, size_t last_read_off) override;

    void read_message(const char* buf, size_t buf_size, message_container& ret) override;

};
}
}