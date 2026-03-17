#pragma once

#include "../core/fd_guard.hpp"
#include <vector>

namespace kasptest::sys::event {


struct PollEvent {
    int fd_;
    bool is_err_;
};

/*
 * @brief wrapper around linux epoll
 *
 * @note i know, that epoll is maybe too much, but my goal was
 * to kill 2 rabbits with one shot. i always wanted to touch
 * this piece of kernel and magic around epoll and nonblocking I/O
 */
struct Poller {
  private:
    core::FdGuard efd_guard_;

  public:
    Poller();

    void subscribe(int fd);

    void unsubscribe(int fd);

    auto wait(int timeout) -> std::vector<PollEvent>;
};


};  // namespace kasptest::sys::event
