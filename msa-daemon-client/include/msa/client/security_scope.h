#pragma once

#include <string>

namespace msa {
namespace client {

struct SecurityScope {
    std::string address;
    std::string policyRef;

    bool operator==(SecurityScope const& s) const {
        return address == s.address && policyRef == s.policyRef;
    }
};

}
}
