#include "fifo.hpp"
#include <cerrno>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

namespace kasptest::sys::ipc {

Fifo::Fifo(const std::string& path)
    : path_(path) {
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

Fifo::~Fifo() {
    if (not path_.empty()) {
        if (unlink(path_.c_str()) < 0) {
            spdlog::warn("unlink failed for fifo file : {}", path_);
        } else {
            spdlog::info("fifo file {} removved", path_);
        }
    }
}

Fifo::Fifo(Fifo&& other) noexcept
    : path_(std::move(other.path_)) {
    other.path_.clear();
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

/* @todo : maybe we don't wanna block here..? */
auto Fifo::open_for_read() -> core::FdGuard {
    int fd = open(path_.c_str(), O_RDONLY /* ?O_NONBLOCK? */);


    if (fd < 0) {
        throw std::runtime_error("open_for_read() failed for : [" + path_ + "]");
    }

    return core::FdGuard(fd);
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

auto Fifo::open_for_write() const -> core::FdGuard {
    int fd = open(path_.c_str(), O_WRONLY);
    if (fd < 0) {
        throw std::runtime_error("open_for write() failed for : [" + path_ + "]");
    }

    return core::FdGuard(fd);
}

auto Fifo::path() const -> std::string_view {
    return path_;
}

}  // namespace kasptest::sys::ipc
