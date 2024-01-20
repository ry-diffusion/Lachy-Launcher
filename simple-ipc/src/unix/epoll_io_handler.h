#pragma once

#include <simpleipc/common/io_handler.h>
#include <unordered_map>
#include <mutex>
#include <thread>

namespace simpleipc {

class epoll_io_handler : public io_handler {

private:
    struct callback_set {
        fd_callback data_cb, close_cb;
    };

    int fd, efd;
    std::unordered_map<int, callback_set> cbs;
    bool running = true;
    std::recursive_mutex cbm;
    std::thread thread;

    void run();

public:
    epoll_io_handler();

    ~epoll_io_handler();


    void add_socket(int fd, fd_callback data_cb, fd_callback close_cb) override;

    void remove_socket(int fd) override;

};

}