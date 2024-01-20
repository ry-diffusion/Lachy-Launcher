#include <simpleipc/common/rpc_call_exception.h>
#include "encoding_json.h"

#include "../connection_internal.h"

using namespace simpleipc;

void encoding::json::send_message(connection_internal& conn, rpc_message const& msg) {
    nlohmann::json data;
    if (msg.has_id())
        data["id"] = msg.id();
    data["method"] = msg.method();
    data["params"] = msg.data();
    send_json(conn, data);
}

void encoding::json::send_message(connection_internal& conn, response_message const& msg) {
    nlohmann::json data;
    if (msg.has_id())
        data["id"] = msg.id();
    data["result"] = msg.data();
    send_json(conn, data);
}

void encoding::json::send_message(connection_internal& conn, error_message const& msg) {
    nlohmann::json data;
    if (msg.has_id())
        data["id"] = msg.id();
    data["error"]["code"] = msg.error_code();
    data["error"]["message"] = msg.error_text();
    data["error"]["data"] = msg.data();
    send_json(conn, data);
}

void encoding::json::send_json(connection_internal& conn, nlohmann::json const& data) {
    std::string buf = data.dump();
    if (buf.length() + 1 > conn.get_max_out_msg_size())
        throw std::runtime_error("Message is too big");
    conn.send_data(buf.data(), buf.size());
    conn.send_data("\n", 1);
}

ssize_t encoding::json::check_read_message_complete(const char* buf, size_t buf_size, size_t last_read_off) {
    for (size_t i = last_read_off; i < buf_size; i++) {
        if (buf[i] == '\n' || buf[i] == '\0')
            return i + 1;
    }
    return -1;
}

void encoding::json::read_message(const char* buf, size_t buf_size, message_container& ret) {
    nlohmann::json data = nlohmann::json::parse(nlohmann::detail::input_adapter(buf, buf_size));
    auto msg_id = data.find("id");
    if (data.count("method") > 0) {
        if (msg_id != data.end() && msg_id->is_number())
            ret.set(rpc_message(*msg_id, data["method"], data["params"]));
        else
            ret.set(rpc_message(data["method"], data["params"]));
    } else if (data.count("error") > 0) {
        auto& error_data = data["error"];
        if (msg_id != data.end() && msg_id->is_number())
            ret.set(error_message(*msg_id, error_data["code"], error_data["message"], error_data["data"]));
        else
            ret.set(error_message(error_data["code"], error_data["message"], error_data["data"]));
    } else if (data.count("result") > 0) {
        if (msg_id != data.end() && msg_id->is_number())
            ret.set(response_message(*msg_id, data["result"]));
        else
            ret.set(response_message(data["result"]));
    } else {
        throw rpc_call_exception(simpleipc::rpc_error_codes::invalid_request, simpleipc::rpc_error_codes::to_string);
    }
}