#include "fifo_guard.hpp"
#include <cerrno>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

namespace kasptest::sys::ipc {

FifoGuard::FifoGuard(const std::string& path)
    : path_(path) {
    /* */
    spdlog::info("[debug] : i'm inside FifoGuard ctor...");

    if (mkfifo(path.c_str(), 0666) < 0) { /* r-w permissions.. */

        if (errno == EEXIST) {
            struct stat st;

            if (stat(path.c_str(), &st) == 0 and S_ISFIFO(st.st_mode)) {
                spdlog::warn("fifo file already exists : {}", path);
            } else {
                throw std::runtime_error("not a fifo file... : " + path);
            }
        } else {
            ///

            throw std::runtime_error("mkfifo() failed... [" + path + "]");
        }
    } else {
        spdlog::info("new fifo file created at : {}", path);
    }
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

FifoGuard::~FifoGuard() {
    if (not path_.empty()) {
        if (unlink(path_.c_str()) < 0) {
            spdlog::warn("unlink failed for fifo file : {}", path_);
        } else {
            spdlog::info("fifo file {} removved", path_);
        }
    }
}

FifoGuard::FifoGuard(FifoGuard&& other) noexcept
    : path_(std::move(other.path_)) {
    other.path_.clear();
}

FifoGuard& FifoGuard::operator=(FifoGuard&& other) noexcept {
    if (this != &other) {
        if (not path_.empty()) {
            unlink(path_.c_str());
        }

        path_ = std::move(other.path_);
        other.path_.clear();
    }
    return *this;
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

/* @note : maybe we don't wanna block here..? */
auto FifoGuard::open_for_read(bool blocking) -> core::FdGuard {
    auto flags = O_RDONLY;
    if (not blocking) {
        flags |= O_NONBLOCK;
    }

    int fd = open(path_.c_str(), flags);


    if (fd < 0) {
        throw std::runtime_error("open_for_read() failed for : [" + path_ + "]");
    }

    return core::FdGuard(fd);
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

auto FifoGuard::open_for_write() const -> core::FdGuard {
    int fd = open(path_.c_str(), O_WRONLY);
    if (fd < 0) {
        throw std::runtime_error("open_for write() failed for : [" + path_ + "]");
    }

    return core::FdGuard(fd);
}

auto FifoGuard::path() const -> std::string_view {
    return path_;
}

}  // namespace kasptest::sys::ipc
