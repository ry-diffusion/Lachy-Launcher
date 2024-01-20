#include "unix_connection.h"

#include <unistd.h>
#include <simpleipc/common/io_handler.h>

using namespace simpleipc;

void unix_connection::register_io_handler() {
    io_handler::get_instance().add_socket(fd, [this](int) { handle_data_available(); }, [this](int) { on_close(); });
}

void unix_connection::unregister_io_handler() {
    io_handler::get_instance().remove_socket(fd);
}

void unix_connection::send_data(const char* data, size_t datalen) {
    if (write(fd, data, datalen) != datalen)
        throw std::runtime_error("write() failed");
}

ssize_t unix_connection::read_data(char* data, size_t datalen) {
    return read(fd, data, datalen);
}
