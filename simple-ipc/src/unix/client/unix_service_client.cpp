#include "unix_service_client.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace simpleipc::client;

void unix_service_client_impl::open(std::string const& path) {
    if (fd != -1)
        throw std::runtime_error("Socket already created (invalid program state)");
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("Failed to open socket: socket() returned " + std::to_string(fd));
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    if (path.length() >= sizeof(addr.sun_path))
        throw std::runtime_error("Path is too long");
    this->path = path;
    strcpy(addr.sun_path, path.c_str());
    if (::connect(fd, (const sockaddr*) &addr, sizeof(addr)) < 0) {
        try {
            close();
        } catch (std::exception&) {}
        throw std::runtime_error("Failed to connect");
    }
    connection = std::make_shared<unix_connection>(fd);
    connection->set_handler(this);
    connection->register_io_handler();
    if (cb)
        cb.load()->connection_opened();
}

void unix_service_client_impl::close() {
    if (fd == -1)
        return;
    if (connection) {
        connection->unregister_io_handler();
        connection.reset();
    }
    shutdown(fd, SHUT_RDWR);
    if (::close(fd) < 0) {
        fd = -1;
        throw std::runtime_error("Failed to close socket");
    }
    fd = -1;
    path = std::string();
}

void unix_service_client_impl::send_message(rpc_message const& msg) {
    if (!connection)
        throw std::runtime_error("No active connection to send the message on");
    connection->send_message(msg);
}

std::unique_ptr<service_client_impl> service_client_impl_factory::create_platform_service() {
    return std::unique_ptr<service_client_impl>(new unix_service_client_impl());
}