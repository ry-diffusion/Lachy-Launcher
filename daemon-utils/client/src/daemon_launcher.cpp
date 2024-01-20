#include <daemon_utils/daemon_launcher.h>

#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <FileUtil.h>
#include <log.h>

#ifdef __APPLE__
#include <chrono>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#else
#include <sys/inotify.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <simpleipc/common/io_handler.h>
#include <future>
#endif

using namespace daemon_utils;

pid_t daemon_launcher::start() {
    std::string cwd = get_cwd();
    std::vector<std::string> args = get_arguments();
    pid_t ret;
    if (!(ret = fork())) {
        setsid();
        chdir(cwd.c_str());
        char** argv = (char**) alloca(sizeof(char*) * (args.size() + 1));
        for (int i = 0; i < args.size(); i++)
            argv[i] = &args[i][0];
        argv[args.size()] = nullptr;
        Log::trace("DaemonLauncher", "Starting daemon: %s", argv[0]);
        int r = execv(argv[0], argv);
        Log::error("DaemonLauncher", "execv error %i", r);
        _exit(1);
    }
    if (ret < 0)
        throw std::runtime_error("fork failed");
    return ret;
}

void daemon_launcher::open(simpleipc::client::service_client_impl& impl) {
    struct stat s;
    stat(service_path.c_str(), &s);
    if (S_ISSOCK(s.st_mode)) {
        // try open
        try {
            impl.open(service_path);
            return;
        } catch (std::exception& e) {
            // open failed
            Log::info("DaemonLauncher", "Daemon file exists, but we could not open it (%s); "
                    "starting the service anyways", e.what());
        }
    }

#ifdef __APPLE__
    remove(service_path.c_str());

    int kq = kqueue();
    std::string dir = FileUtil::getParent(service_path);
    FileUtil::mkdirRecursive(dir);
    int f = ::open(dir.c_str(), O_RDONLY);

    pid_t proc = start();

    struct kevent k_ev[2];
    EV_SET(&k_ev[0], f, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);
    EV_SET(&k_ev[1], proc, EVFILT_PROC, EV_ADD | EV_CLEAR, NOTE_EXIT, 0, NULL);
    kevent(kq, k_ev, 2, NULL, 0, NULL);

    struct kevent ev_set;
    struct kevent ev_list[2];
    int n;

    auto wait_until = std::chrono::system_clock::now() + std::chrono::seconds(10);

    // If the process dies or a file is created, open
    while(true) {
        long long remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                wait_until - std::chrono::system_clock::now()).count();
        if (remaining <= 50) // exit the loop anyways if we're only 50ms from reaching the end time
            break;

        struct timespec timeout;
        timeout.tv_sec = (int) remaining / 1000;
        timeout.tv_nsec =  (int) (remaining % 1000) * 1000;
        n = kevent(kq, NULL, 0, ev_list, 2, &timeout);
        bool should_exit = false;
        for(int i = 0; i < n; i++) {
            if (ev_list[i].fflags & NOTE_EXIT) { // Process exited
                should_exit = true;
                break;
            }
            if (ev_list[i].fflags & NOTE_WRITE) { // Change happened in the directory
                if (access(service_path.c_str(), F_OK) == 0) {
                    should_exit = true;
                    break;
                }
            }
        }
        if (should_exit)
            break;
    }

    close(f);
    close(kq);
#else
    // Start the service and wait for the service file to show up
    int fd = inotify_init();
    if (fd < 0)
        throw std::runtime_error("inotify_init failed");
    std::string dir = FileUtil::getParent(service_path);
    FileUtil::mkdirRecursive(dir);
    int wd = inotify_add_watch(fd, dir.c_str(), IN_CREATE);
    if (wd < 0)
        throw std::runtime_error("inotify_add_watch failed");
    std::string service_filename;
    {
        auto iof = service_path.rfind('/');
        if (iof != std::string::npos)
            service_filename = service_path.substr(iof + 1);
        else
            service_filename = service_path;
    }

    // Try to open the file again, in case the service was stated in the meanwhile
    stat(service_path.c_str(), &s);
    if (S_ISSOCK(s.st_mode)) {
        try {
            impl.open(service_path);
            close(fd);
            return;
        } catch (std::exception& e) {
        }
    }

    pid_t proc = start();

    std::promise<void> stop_promise;
    auto stop_future = stop_promise.get_future();

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    int sfd = signalfd(-1, &mask, SFD_NONBLOCK);
    simpleipc::io_handler::get_instance().add_socket(sfd, [proc, &stop_promise](int fd) {
        int status;
        if (waitpid(proc, &status, WNOHANG) == -1)
            return;
        stop_promise.set_value();
    }, [&stop_promise](int fd) {
        stop_promise.set_value();
    });
    simpleipc::io_handler::get_instance().add_socket(fd, [wd, &stop_promise, service_filename](int fd) {
        unsigned int av;
        ioctl(fd, FIONREAD, &av);
        bool bufs = av < 1024 * 16;
        char* buf = (char*) (bufs ? alloca(av) : malloc(av));
        int n = read(fd, buf, av);
        if (n != av) {
            Log::warn("DaemonLauncher", "Didn't read exactly the event size");
            if (!bufs)
                free(buf);
            return;
        }
        size_t o = 0;
        while (o < av) {
            inotify_event& event = *((inotify_event*) &buf[o]);
            if (event.wd == wd) {
                if (strncmp(event.name, service_filename.c_str(), event.len) == 0)
                    stop_promise.set_value();
            }
            o += sizeof(inotify_event) + event.len;
        }
        if (!bufs)
            free(buf);
    }, [&stop_promise](int fd) {
        stop_promise.set_value();
    });

    stop_future.wait_for(std::chrono::seconds(10));
    simpleipc::io_handler::get_instance().remove_socket(sfd);
    simpleipc::io_handler::get_instance().remove_socket(fd);
    close(sfd);
    close(fd);
#endif

    impl.open(service_path);
}
