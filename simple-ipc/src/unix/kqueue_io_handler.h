#pragma once

#include <simpleipc/common/io_handler.h>

#include <mutex>
#include <thread>
#include <unordered_map>

namespace simpleipc
{

  class kqueue_io_handler : public io_handler
  {
   private:
    struct callback_set
    {
      fd_callback data_cb, close_cb;
    };

    int kq;
    int stop_pipe[2];
    std::unordered_map<int, callback_set> cbs;
    bool running = true;
    std::recursive_mutex cbm;
    std::thread thread;

    void run();

   public:
    kqueue_io_handler();

    ~kqueue_io_handler();

    void add_socket(int fd, fd_callback data_cb, fd_callback close_cb) override;

    void remove_socket(int fd) override;
  };

}  // namespace simpleipc