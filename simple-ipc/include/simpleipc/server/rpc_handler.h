#pragma once

#include <functional>
#include <nlohmann/json.hpp>
#include "../common/rpc_result.h"

namespace simpleipc {

class rpc_message;
class connection;

namespace server {

class rpc_handler {

public:
    using result_handler = std::function<void (rpc_json_result)>;

    using call_handler_async = std::function<void (connection& conn, std::string const& method, nlohmann::json const& data, result_handler const&)>;

    using call_handler_sync = std::function<rpc_json_result (connection& conn, std::string const& method, nlohmann::json const& data)>;

private:
    std::map<std::string, call_handler_async> handlers;

public:
    void add_handler_async(std::string const& method, call_handler_async handler);

    void add_handler(std::string const& method, call_handler_sync handler) {
        add_handler_async(method, [handler](connection& conn, std::string const& method, nlohmann::json const& data, result_handler const& result) {
            result(handler(conn, method, data));
        });
    }

    void invoke(connection& conn, std::string const& method, nlohmann::json const& data, result_handler const& handler);

    void invoke(std::shared_ptr<connection> conn, rpc_message const& msg);

};

}

}