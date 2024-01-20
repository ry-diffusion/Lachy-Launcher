#include "default_rpc_handler.h"

#include <simpleipc/common/version.h>
#include "../common/encoding/encoding.h"
#include "../common/encoding/encodings.h"
#include "../common/connection_internal.h"

using namespace simpleipc::server;

default_rpc_handler::default_rpc_handler() {
    using namespace std::placeholders;
    add_handler_async(".hello", std::bind(&default_rpc_handler::handle_hello, this, _1, _3, _4));
}

void default_rpc_handler::handle_hello(connection& conn, nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    auto encoding = encoding::encodings::get_default_encoding();
    for (auto const& encoding_name : data.at("encodings")) {
        auto alt_enc = encoding::encodings::get_encoding_by_name(encoding_name);
        if (alt_enc != nullptr) {
            encoding = alt_enc;
            break;
        }
    }
    handler(rpc_json_result::response({
            {"version", version::current_version},
            {"encoding", encoding->name()}
    }));
    ((connection_internal&) conn).set_encoding(encoding);
}