#pragma once

#include <string>
#include <vector>

namespace cll {
namespace http {

struct HttpResponse {

    long status;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;


    std::string findHeader(std::string const& name) {
        for (auto const& h : headers) {
            if (h.first == name)
                return h.second;
        }
        return std::string();
    }

};

}
}