#pragma once

#include <functional>

namespace simpleipc {

class io_handler {

public:
    using fd_callback = std::function<void (int fd)>;

    static io_handler& get_instance();


    virtual void add_socket(int fd, fd_callback data_cb, fd_callback close_cb) = 0;

    virtual void remove_socket(int fd) = 0;

};

}