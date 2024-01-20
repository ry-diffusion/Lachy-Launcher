#pragma once

#include <simpleipc/server/rpc_handler.h>

namespace simpleipc {
namespace server {

class default_rpc_handler : public rpc_handler {

public:
    default_rpc_handler();

private:
    void handle_hello(connection& conn, nlohmann::json const& data, rpc_handler::result_handler const& handler);

};

}
}