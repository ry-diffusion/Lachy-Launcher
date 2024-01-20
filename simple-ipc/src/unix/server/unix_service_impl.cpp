#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <simpleipc/common/io_handler.h>
#include "unix_service_impl.h"
#include "../common/unix_connection.h"

using namespace simpleipc::server;

unix_service_impl::~unix_service_impl() {
    for (auto& conn : connections) {
        conn->unregister_io_handler();
    }
}

void unix_service_impl::bind(std::string const& path) {
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
    unlink(path.c_str());
    if (::bind(fd, (const sockaddr*) &addr, sizeof(addr)) < 0) {
        try {
            close();
        } catch (std::exception&) {}
        throw std::runtime_error("Failed to bind socket");
    }
    if (listen(fd, 32) < 0) {
        try {
            close();
        } catch (std::exception&) {}
        throw std::runtime_error("Failed to start listening on socket");
    }
    io_handler::get_instance().add_socket(fd, [this](int) { handle_incoming(); }, [this](int) { close(); });
}

void unix_service_impl::close() {
    if (fd == -1)
        return;
    io_handler::get_instance().remove_socket(fd);
    shutdown(fd, SHUT_RDWR);
    if (::close(fd) < 0) {
        fd = -1;
        throw std::runtime_error("Failed to close socket");
    }
    fd = -1;
    unlink(path.c_str());
    path = std::string();
}

void unix_service_impl::handle_incoming() {
    sockaddr_storage ss;
    socklen_t ss_len = sizeof(ss);
    int fd = accept(this->fd, (sockaddr*) &ss, &ss_len);
    if (fd < 0) { // failed
#ifdef SIMPLEIPC_DEBUG_LOGGING
        printf("accept() failed\n");
#endif
        return;
    }
    std::shared_ptr<unix_connection> conn (new unix_connection(fd));
    conn->set_handler(this);
    conn->register_io_handler();
    connections.insert(conn);
    cb->on_client_connected(*conn);
}

void unix_service_impl::connection_closed(connection& conn) {
    auto unix_conn = std::dynamic_pointer_cast<unix_connection>(conn.shared_from_this());
    unix_conn->unregister_io_handler();
    shutdown(unix_conn->get_fd(), SHUT_RDWR);
    ::close(unix_conn->get_fd());
    connections.erase(unix_conn);
    cb->on_client_disconnected(*unix_conn);
}

std::unique_ptr<service_impl> service_impl_factory::create_platform_service() {
    return std::unique_ptr<service_impl>(new unix_service_impl());
}