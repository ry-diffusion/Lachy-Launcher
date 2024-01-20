#pragma once

#include <nlohmann/json.hpp>
#include "message_id.h"
#include "error_code.h"

namespace simpleipc {

class error_message : public message_with_id {

private:
    rpc_error_code _error_code;
    std::string _error_text;
    nlohmann::json _data;

public:
    error_message() : _error_code(0) {}

    error_message(rpc_error_code error_code, std::string error_text, nlohmann::json data) :
            _error_code(error_code), _error_text(std::move(error_text)), _data(std::move(data)) {}

    error_message(message_id id, rpc_error_code error_code, std::string error_text, nlohmann::json data) :
            message_with_id(id), _error_code(error_code), _error_text(std::move(error_text)),
            _data(std::move(data)) {}


    inline rpc_error_code error_code() const { return _error_code; }

    inline std::string const& error_text() const { return _error_text; }

    inline nlohmann::json const& data() const { return _data; }
};

}
