#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace msa {
namespace client {

struct BaseAccountInfo {

private:

    std::string username;
    std::string cid;

public:

    BaseAccountInfo(std::string username, std::string cid) : username(std::move(username)), cid(std::move(cid)) {}

    std::string const& getUsername() const { return username; }

    std::string const& getCID() const { return cid; }


    static BaseAccountInfo fromJson(nlohmann::json const& j) {
        return BaseAccountInfo(j["username"], j["cid"]);
    }

};

}
}