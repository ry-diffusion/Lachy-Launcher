#pragma once

#include "../common/rpc_result.h"

namespace simpleipc {
namespace client {

class service_client;

using rpc_json_result_callback = std::function<void (rpc_json_result)>;

class rpc_json_call {

private:
    service_client& client;
    std::string method;
    nlohmann::json data;

public:
    rpc_json_call(service_client& client, std::string method, nlohmann::json data) : client(client),
                                                                                method(std::move(method)),
                                                                                data(std::move(data)) {}

    /**
     * Executes this call synchronically, returning the result.
     */
    rpc_json_result call();

    /**
     * Executes this call asynchronically, returning the result.
     */
    void call(rpc_json_result_callback cb);

    /**
     * Executes this call asynchronically, ignoring the result.
     */
    void run();

};

}
}