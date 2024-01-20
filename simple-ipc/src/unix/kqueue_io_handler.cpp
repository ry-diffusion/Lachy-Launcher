#include "kqueue_io_handler.h"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

using namespace simpleipc;

kqueue_io_handler::kqueue_io_handler() {
    kq = kqueue();

    pipe(stop_pipe);

    struct kevent k_set;
    EV_SET(&k_set, stop_pipe[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &k_set, 1, NULL, 0, NULL);

    thread = std::thread(std::bind(&kqueue_io_handler::run, this));
}

kqueue_io_handler::~kqueue_io_handler() {
    cbm.lock();
    running = false;
    cbm.unlock();
    char c = 0;
    write(stop_pipe[1], &c, 1);
    thread.join();
    close(kq);
    close(stop_pipe[0]);
    close(stop_pipe[1]);
}

void kqueue_io_handler::add_socket(int fd, fd_callback data_cb, fd_callback close_cb) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    cbm.lock();
    cbs[fd] = {data_cb, close_cb};
    cbm.unlock();

    struct kevent k_set;
    EV_SET(&k_set, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &k_set, 1, NULL, 0, NULL);
}

void kqueue_io_handler::remove_socket(int fd) {
    struct kevent k_set;
    EV_SET(&k_set, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(kq, &k_set, 1, NULL, 0, NULL);

    cbm.lock();
    cbs.erase(fd);
    cbm.unlock();
}

void kqueue_io_handler::run() {
    struct kevent ev_set;
    struct kevent ev_list[64];
    int n;

    while (true) {
        n = kevent(kq, NULL, 0, ev_list, 64, NULL);
        cbm.lock();
        for (int i = 0; i < n; i++) {
            auto cb = cbs.find(ev_list[i].ident);
            if (ev_list[i].ident == stop_pipe[1] && (ev_list[i].filter & EVFILT_READ))
                break;
            if (cb == cbs.end())
                continue;

            if (ev_list[i].flags & EV_EOF) {
                if (cb->second.close_cb)
                    cb->second.close_cb(ev_list[i].ident);
            } else if (ev_list[i].filter & EVFILT_READ) {
                if (cb->second.data_cb)
                    cb->second.data_cb(ev_list[i].ident);
            }
        }

        if (!running) {
            cbm.unlock();
            break;
        }
        cbm.unlock();
    }
}

io_handler& io_handler::get_instance() {
    static kqueue_io_handler instance;
    return instance;
}