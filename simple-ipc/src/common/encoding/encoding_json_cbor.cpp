#include "encoding_json_cbor.h"
#include "varint.h"

#include "../connection_internal.h"

using namespace simpleipc;

void encoding::json_cbor::send_message(connection_internal& conn, rpc_message const& msg) {
    send_json(conn, {get_id_json(msg), msg.method(), msg.data()});
}

void encoding::json_cbor::send_message(connection_internal& conn, response_message const& msg) {
    send_json(conn, {get_id_json(msg), msg.data()});
}

void encoding::json_cbor::send_message(connection_internal& conn, error_message const& msg) {
    send_json(conn, {get_id_json(msg), msg.error_code(), msg.error_text(), msg.data()});
}

void encoding::json_cbor::send_json(connection_internal& conn, nlohmann::json const& data) {
    auto buf = nlohmann::json::to_cbor(data);

    char vbuf[10];
    size_t vbufs;
    varint::encode_unsigned(buf.size(), vbuf, vbufs);
    conn.send_data(vbuf, vbufs);
    conn.send_data((char*) buf.data(), buf.size());
}

ssize_t encoding::json_cbor::check_read_message_complete(const char* buf, size_t buf_size, size_t last_read_off) {
    size_t varint_size = buf_size;
    unsigned long long size;
    if (!varint::try_decode_unsigned(buf, buf_size, size, &varint_size))
        return -1;
    size_t total_size = size + varint_size;
    if (buf_size >= total_size)
        return total_size;
    return -1;
}

void encoding::json_cbor::read_message(const char* buf, size_t buf_size, message_container& ret) {
    size_t varint_size = buf_size;
    auto size = varint::decode_unsigned(buf, buf_size, &varint_size);
    nlohmann::json data =
            nlohmann::json::from_cbor(nlohmann::detail::input_adapter(&buf[varint_size], buf_size - varint_size));

    if (data.size() == 3) { // rpc request
        if (data[0].is_number())
            ret.set(rpc_message(data[0], data[1], data[2]));
        else
            ret.set(rpc_message(data[1], data[2]));
    } else if (data.size() == 2) { // response
        if (data[0].is_number())
            ret.set(response_message(data[0], data[1]));
        else
            ret.set(response_message(data[1]));
    } else { // error (4)
        if (data[0].is_number())
            ret.set(error_message(data[0], data[1], data[2], data[3]));
        else
            ret.set(error_message(data[1], data[2], data[3]));
    }
}