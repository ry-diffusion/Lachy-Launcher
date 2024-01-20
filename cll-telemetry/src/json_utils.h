#pragma once

#include <nlohmann/json.hpp>

namespace cll {

class JsonUtils {

public:
    static int asInt(nlohmann::json const& val) {
        if (val.is_number())
            return val.get<int>();
        else
            return std::stoi(val.get<std::string>());
    }

};

}