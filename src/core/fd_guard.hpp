#pragma once

#include <cstdlib>
#include <unistd.h>

namespace kasptest::core {

/* @brief RAII wrapper of the file descriptor */
class FdGuard {
  private:
    int fd_ = -1;

  public:
    FdGuard() = default;

    explicit FdGuard(int fd)
        : fd_(fd) {}

    ~FdGuard() {
        if (fd_ >= 0) {
            ::close(fd_);
        } else {
            /*
             * i'm not sure about that..
             *
             * exit(EXIT_FAILURE);
             */
        }
    }

    /* non-movable */
    FdGuard& operator=(const FdGuard& oth) = delete;
    FdGuard(const FdGuard& oth) = delete;

    FdGuard(FdGuard&& oth) noexcept
        : fd_(oth.fd_) {
        oth.fd_ = -1;
    }

    FdGuard& operator=(FdGuard&& oth) noexcept {
        if (this != &oth) {
            if (fd_ >= 0) {
                ::close(fd_);
            }
            fd_ = oth.fd_;
            oth.fd_ = -1;
        }
        return *this;
    }

    int get_fd() const noexcept {
        return fd_;
    }

    bool valid() const noexcept {
        return fd_ >= 0;
    }

    /* @brief release ownership od the fd */
    int release() {
        int fd = fd_;

        fd_ = -1;
        return fd;
    }
};

}  // namespace kasptest::core
