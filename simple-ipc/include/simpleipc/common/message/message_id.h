#pragma once

#include <nlohmann/json.hpp>

namespace simpleipc {

using message_id = long long;

class message_with_id {

private:
    bool _has_id;
    message_id _id;

protected:
    message_with_id() : _has_id(false), _id(0) {}

    message_with_id(message_id id) : _has_id(true), _id(id) {}

public:
    message_id id() const { return _id; }

    bool has_id() const { return _has_id; }

};

}
