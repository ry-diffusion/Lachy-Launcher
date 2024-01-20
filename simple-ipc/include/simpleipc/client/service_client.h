#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "../common/connection.h"
#include "../common/message/message_id.h"
#include "service_client_impl.h"
#include "rpc_json_call.h"

namespace simpleipc {
namespace client {

class service_client : private service_client_impl::callback_interface {

private:
    friend class rpc_json_call;

    std::unique_ptr<service_client_impl> impl;
    std::atomic<message_id> next_message_id;
    std::recursive_mutex cb_mutex;
    std::unordered_map<message_id, rpc_json_result_callback> cbs;
    bool hello_response_value = false;
    std::mutex hello_response_mutex;
    std::condition_variable hello_response_cv;

    void handle_message(response_message const& msg) override;

    void handle_message(error_message const& msg) override;


    void send_message(rpc_message const& msg);

    void add_rpc_cb(message_id msg, rpc_json_result_callback cb);

    rpc_json_result_callback get_rpc_cb(message_id msg);

protected:
    inline service_client_impl& get_impl() {
        return *impl;
    }
    inline service_client_impl::callback_interface& get_cb_interface() {
        return *this;
    }

    void connection_opened() override;

    void connection_closed() override;

public:
    service_client(std::unique_ptr<service_client_impl> impl) : impl(std::move(impl)), next_message_id(0) {
        this->impl->set_callback_interface(this);
    }
    service_client(std::string const& path) : service_client(service_client_impl_factory::create_platform_service()) {
        impl->open(path);
        wait_for_hello_message();
    }
    virtual ~service_client() {
        impl->set_callback_interface(nullptr);
        impl->close();
    }

    rpc_json_call rpc(std::string method, nlohmann::json data) {
        return rpc_json_call(*this, std::move(method), std::move(data));
    }


    void send_hello_message();

    void wait_for_hello_message();

};

}
}