#pragma once

namespace simpleipc {

using rpc_error_code = int;

using rpc_error_string_translator = const char* (*)(rpc_error_code);

namespace rpc_error_codes {

static constexpr rpc_error_code success = 0;
static constexpr rpc_error_code parse_error = -32700;
static constexpr rpc_error_code invalid_request = -32600;
static constexpr rpc_error_code method_not_found = -32601;
static constexpr rpc_error_code invalid_params = -32602;
static constexpr rpc_error_code internal_error = -32603;
static constexpr rpc_error_code connection_closed = -32000;
static constexpr rpc_error_code no_hello_reply = -32001;

const char* to_string(rpc_error_code code);

}

}