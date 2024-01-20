#pragma once

#include <nlohmann/json.hpp>
#include "message_id.h"

namespace simpleipc {

class response_message : public message_with_id {

private:
    nlohmann::json _data;

public:
    response_message() {}

    explicit response_message(nlohmann::json data) : _data(std::move(data)) {}

    response_message(message_id id, nlohmann::json data) : message_with_id(id), _data(std::move(data)) {}


    inline nlohmann::json const& data() const { return _data; }
};

}
