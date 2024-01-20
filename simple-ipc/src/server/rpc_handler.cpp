#include <simpleipc/server/rpc_handler.h>
#include <simpleipc/common/connection.h>
#include <simpleipc/common/message/rpc_message.h>
#include <simpleipc/common/message/response_message.h>
#include <simpleipc/common/message/error_message.h>
#include <simpleipc/common/rpc_call_exception.h>

using namespace simpleipc::server;

void rpc_handler::add_handler_async(std::string const& method, call_handler_async handler) {
    if (handlers.count(method) > 0)
        throw std::runtime_error("A handler for this method already exists");
    handlers[method] = std::move(handler);
}

void rpc_handler::invoke(connection& conn, std::string const& method, nlohmann::json const& data,
                         result_handler const& handler) {
    auto h = handlers.find(method);
    if (h == handlers.end()) {
        handler(rpc_json_result::error(rpc_error_codes::method_not_found, rpc_error_codes::to_string));
        return;
    }
    try {
        h->second(conn, method, data, std::move(handler));
    } catch (rpc_call_exception_interface& e) {
        handler(rpc_json_result::error(e.code(), e.what(), e.data()));
    } catch (std::exception& e) {
        handler(rpc_json_result::error(rpc_error_codes::internal_error, rpc_error_codes::to_string));
    }
}

void rpc_handler::invoke(std::shared_ptr<connection> conn, rpc_message const& msg) {
    if (msg.has_id()) {
        message_id id = msg.id();
        std::weak_ptr<connection> conn_weak(conn);
        invoke(*conn, msg.method(), msg.data(), [conn_weak, id](rpc_json_result result) {
            auto conn_ptr = conn_weak.lock();
            if (!conn_ptr)
                return;
            if (result.success())
                conn_ptr->send_message(response_message(id, std::move(result._data)));
            else
                conn_ptr->send_message(error_message(id, result._error_code, std::move(result._error_text),
                                                      std::move(result._data)));
        });
    } else {
        invoke(*conn, msg.method(), msg.data(), [](rpc_json_result) {});
    }
}