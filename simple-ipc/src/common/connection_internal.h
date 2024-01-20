#pragma once

#include <simpleipc/common/connection.h>
#include <simpleipc/common/connection_handler.h>
#include <mutex>
#include <atomic>
#include "encoding/encoding.h"
#include "encoding/encodings.h"

namespace simpleipc {

class connection_internal : public connection {

public:
    using message_callback = std::function<void (std::string const& method, nlohmann::json const& data)>;
    using close_callback = std::function<void ()>;

private:
    static const size_t BUFFER_SIZE = 1024 * 16;
    static const size_t MAX_BUFFER_SIZE = 1024 * 512;

    connection_handler* handler;
    std::vector<char> buffer;
    size_t buffer_start_off = 0;
    size_t buffer_off = 0;
    std::atomic<encoding::encoding*> current_encoding;
    size_t max_out_msg_size = MAX_BUFFER_SIZE;
    message_container current_message;

public:
    connection_internal() : buffer(BUFFER_SIZE), current_encoding(encoding::encodings::get_default_encoding()) {
    }

    inline size_t get_max_out_msg_size() const {
        return max_out_msg_size;
    }

    void set_handler(connection_handler* handler) {
        this->handler = handler;
    }

    void set_encoding(encoding::encoding* enc) {
        current_encoding = enc;
    }

    virtual void register_io_handler() = 0;

    virtual void unregister_io_handler() = 0;

    virtual void send_data(const char* data, size_t datalen) = 0;

    virtual ssize_t read_data(char* data, size_t datalen) = 0;


    virtual void send_message(rpc_message const& msg) {
#ifdef SIMPLEIPC_DEBUG_LOGGING
        printf("Send rpc message: method=%s, data=%s\n", msg.method().c_str(), msg.data().dump().c_str());
#endif
        current_encoding.load()->send_message(*this, msg);
    }

    virtual void send_message(response_message const& msg) {
#ifdef SIMPLEIPC_DEBUG_LOGGING
        printf("Send response message: data=%s\n", msg.data().dump().c_str());
#endif
        current_encoding.load()->send_message(*this, msg);
    }

    virtual void send_message(error_message const& msg) {
#ifdef SIMPLEIPC_DEBUG_LOGGING
        printf("Send error message: code=%i, message=%s, data=%s\n", msg.error_code(), msg.error_text().c_str(),
               msg.data().dump().c_str());
#endif
        current_encoding.load()->send_message(*this, msg);
    }

protected:
    void on_message(message_container const& msg);

    void on_close() {
        handler->connection_closed(*this);
    }

    void on_error(rpc_call_exception_interface const& ex_as_rpc, std::exception_ptr ex_ptr = std::current_exception()) {
        handler->handle_message_parse_error(*this, ex_ptr, ex_as_rpc);
    }


    void handle_data_available();

};

}