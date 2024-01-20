#include "epoll_io_handler.h"

#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

using namespace simpleipc;

epoll_io_handler::epoll_io_handler() {
    fd = epoll_create1(0);
    if (fd < 0)
        throw std::runtime_error("Failed to create epoll fd");

    efd = eventfd(0, EFD_NONBLOCK);

    epoll_event event;
    event.data.fd = efd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(fd, EPOLL_CTL_ADD, efd, &event);

    thread = std::thread(std::bind(&epoll_io_handler::run, this));
}

epoll_io_handler::~epoll_io_handler() {
    cbm.lock();
    running = false;
    cbm.unlock();
    uint64_t b = 1;
    write(efd, &b, sizeof(b));
    thread.join();
    close(fd);
}

void epoll_io_handler::add_socket(int fd, fd_callback data_cb, fd_callback close_cb) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    cbm.lock();
    cbs[fd] = {data_cb, close_cb};
    cbm.unlock();

    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLET;
    epoll_ctl(this->fd, EPOLL_CTL_ADD, fd, &event);
}

void epoll_io_handler::remove_socket(int fd) {
    cbm.lock();
    cbs.erase(fd);
    cbm.unlock();

    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLET;
    epoll_ctl(this->fd, EPOLL_CTL_DEL, fd, &event);
}

void epoll_io_handler::run() {
    const size_t event_count = 64;
    epoll_event* events = new epoll_event[event_count];
    int n;
    while (true) {
        n = epoll_wait(fd, events, event_count, -1);
        cbm.lock();
        for (int i = 0; i < n; i++) {
            auto cb = cbs.find(events[i].data.fd);
            if (cb == cbs.end())
                continue;
            if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP)) {
                if (cb->second.close_cb)
                    cb->second.close_cb(events[i].data.fd);
            } else if (events[i].events & EPOLLIN) {
                if (cb->second.data_cb)
                    cb->second.data_cb(events[i].data.fd);
            }
        }
        if (!running) {
            cbm.unlock();
            break;
        }
        cbm.unlock();
    }
    delete[] events;
}

io_handler& io_handler::get_instance() {
    static epoll_io_handler instance;
    return instance;
}