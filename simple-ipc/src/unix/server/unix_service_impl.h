#pragma once

#include <unordered_set>
#include <memory>
#include "../common/unix_connection.h"
#include "../../server/base_service_impl.h"

namespace simpleipc {
namespace server {

class unix_service_impl : public base_service_impl {

private:
    int fd = -1;
    std::string path;
    std::unordered_set<std::shared_ptr<unix_connection>> connections;

    void handle_incoming();

public:
    ~unix_service_impl();

    void bind(std::string const& path) override;

    void close() override;

    void connection_closed(connection& conn) override;

};

}
}