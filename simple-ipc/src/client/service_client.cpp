#include <simpleipc/client/service_client.h>
#include <simpleipc/common/message/response_message.h>
#include <simpleipc/common/message/error_message.h>
#include <simpleipc/common/version.h>
#include "../common/encoding/encodings.h"
#include "../common/connection_internal.h"

using namespace simpleipc::client;

void service_client::add_rpc_cb(message_id msg, rpc_json_result_callback cb) {
    std::lock_guard<std::recursive_mutex> lock(cb_mutex);
    cbs[msg] = cb;
}

void service_client::connection_opened() {
    send_hello_message();
}

void service_client::connection_closed() {
    hello_response_mutex.lock();
    hello_response_value = false;
    hello_response_mutex.unlock();
    cb_mutex.lock();
    auto cbs = std::move(this->cbs);
    cb_mutex.unlock();
    for (auto& cb : cbs)
        cb.second(rpc_json_result::error(rpc_error_codes::connection_closed, rpc_error_codes::to_string));
}

void service_client::send_message(rpc_message const& msg) {
    std::unique_lock<std::mutex> lock(hello_response_mutex);
    if (!hello_response_value && msg.method() != ".hello") {
        lock.unlock();
        auto cb = get_rpc_cb(msg.id());
        if (cb)
            cb(rpc_json_result::error(simpleipc::rpc_error_codes::no_hello_reply, "Haven't got hello reply yet"));
        return;
    }
    lock.unlock();
    impl->send_message(msg);
}

rpc_json_result_callback service_client::get_rpc_cb(message_id msg) {
    std::lock_guard<std::recursive_mutex> lock(cb_mutex);
    auto it = cbs.find(msg);
    if (it != cbs.end()) {
        auto val = it->second;
        cbs.erase(it);
        return std::move(val);
    }
    return nullptr;
}

void service_client::handle_message(response_message const& msg) {
    if (!msg.has_id())
        return;
    auto cb = get_rpc_cb(msg.id());
    if (cb)
        cb(rpc_json_result::response(msg.data()));
}

void service_client::handle_message(error_message const& msg) {
    if (!msg.has_id())
        return;
    auto cb = get_rpc_cb(msg.id());
    if (cb)
        cb(rpc_json_result::error(msg.error_code(), msg.error_text(), msg.data()));
}

void service_client::send_hello_message() {
    rpc(".hello", {
            {"version", version::current_version},
            {"encodings", encoding::encodings::get_preferred_encodings()}
    }).call([this](rpc_json_result res) {
        if (res.success()) {
            auto conn = impl->get_connection();
            std::string enc = res.data().at("encoding");
            if (conn != nullptr)
                ((connection_internal*) conn)->set_encoding(
                        encoding::encodings::get_encoding_by_name(res.data().at("encoding")));
        }

        std::unique_lock<std::mutex> lock (hello_response_mutex);
        hello_response_value = true;
        hello_response_cv.notify_all();
    });
}

void service_client::wait_for_hello_message() {
    std::unique_lock<std::mutex> lock (hello_response_mutex);
    hello_response_cv.wait(lock, [this]() { return hello_response_value; });
}