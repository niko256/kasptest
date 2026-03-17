#include "poller.hpp"
#include <cerrno>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <vector>

namespace kasptest::sys::event {

/*
 * https://suchprogramming.com/epoll-in-3-easy-steps/
 * https://jvns.ca/blog/2017/06/03/async-io-on-linux--select--poll--and-epoll/
 */

const static int MAX_EVENTS = 64;

Poller::Poller() {
    auto epoll_fd = epoll_create1(0);

    if (epoll_fd < 0) {
        throw std::runtime_error("epoll_create(1) failed...");
    }

    efd_guard_ = core::FdGuard(epoll_fd);
    /* */
    spdlog::trace("[Poller] : epoll instance created with fd:={}", efd_guard_.get_fd());
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

void Poller::subscribe(int fd) {
    epoll_event ev{};
    ev.events = EPOLLIN;

    ev.data.fd = fd;

    if (epoll_ctl(efd_guard_.get_fd(), EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl(ADD) failed...");
    }

    spdlog::trace("[Poller] : subscribed on fd:={}", fd);
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

void Poller::unsubscribe(int fd) {
    epoll_event ev{};


    if (epoll_ctl(efd_guard_.get_fd(), EPOLL_CTL_DEL, fd, &ev) < 0) {
        spdlog::warn("[Poller] : epoll_ctl(DEL) failed for fd:={}", fd);
        /* throw std::runtime_error("epoll_ctl(DEL) failed..."); */

    } else {
        spdlog::trace("[Poller] : unsubscribed from fd:={}", fd);
    }
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

auto Poller::wait(int timeout) -> std::vector<PollEvent> {
    epoll_event events[MAX_EVENTS];


    int num_evnts = epoll_wait(efd_guard_.get_fd(), events, MAX_EVENTS, timeout);

    if (num_evnts < 0) {
        if (errno == EINTR)
            return {};
    }

    throw std::runtime_error("epoll_wait() failed...");

    std::vector<PollEvent> res;
    res.reserve(num_evnts);
    for (int i = 0; i < num_evnts; ++i) {
        PollEvent ev;
        ev.fd_ = events[i].data.fd;

        bool has_error = (events[i].events & EPOLLERR) != 0;
        ev.is_err_ = has_error;

        res.push_back(ev);
    }
    return res;
}

}  // namespace kasptest::sys::event
