#pragma once

#include <nlohmann/json.hpp>
#include "message_id.h"

namespace simpleipc {

class rpc_message : public message_with_id {

private:
    std::string _method;
    nlohmann::json _data;

public:
    rpc_message() {}

    rpc_message(std::string method, nlohmann::json data) :
            _method(std::move(method)), _data(std::move(data)) {}

    rpc_message(message_id id, std::string method, nlohmann::json data) :
            message_with_id(id), _method(std::move(method)), _data(std::move(data)) {}


    inline std::string const& method() const { return _method; }

    inline nlohmann::json const& data() const { return _data; }
};

}
