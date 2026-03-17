#include "pipe_sink.hpp"
#include "../core/fd_guard.hpp"
#include <cerrno>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include "../utils/repr.hpp"

namespace kasptest::sys::ipc {

PipeResultSink::PipeResultSink() {
    int fds[2];

    if (pipe(fds) < 0) {
        throw std::runtime_error("pipe failed...");
    }

    read_end_ = core::FdGuard(fds[0]);
    write_end_ = core::FdGuard(fds[1]);


    int flags = fcntl(read_end_.get_fd(), F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl() failed... [F_GETFL]");
    }

    if (fcntl(read_end_.get_fd(), F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("fnctl() failed... [F_SETFL, O_NONBLOCCK]");
    }

    spdlog::trace(
        "pipe created : [reader_fd:={}], [writer_fd:={}]",
        read_end_.get_fd(),
        write_end_.get_fd()
    );
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

void PipeResultSink::send(const core::ScanResult& res) {
    std::string json = utils::repr::serialize_result(res);

    json += "\n";

    auto total = 0;
    while (total < json.size()) {
        auto written
            = write(write_end_.get_fd(), json.data() + total, json.size() - total);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                spdlog::error("write() to pipe failed...");
                break;
            }
        }
        total += written;
    }
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

std::vector<core::ScanResult> PipeResultSink::collect() {
    std::vector<core::ScanResult> res;
    char buff[4096];

    while (true) {
        auto bytes_r = read(read_end_.get_fd(), buff, sizeof(buff));

        if (bytes_r < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }

            spdlog::error("read() from pipe failed...");
            break;
        } else if (bytes_r == 0) {
            break;
        }
        read_buff_.append(buff, bytes_r);
    }

    size_t pos = 0;
    while (pos = read_buff_.find('\n') != std::string::npos) {
        std::string json_line = read_buff_.substr(0, pos);

        read_buff_.erase(0, pos + 1);

        if (not json_line.empty()) {
            try {
                res.push_back(utils::repr::deserialize_result(json_line));
            } catch (const std::exception& er) {
                spdlog::error("failed to parse result from pipe... : {}", er.what());
            }
        }
    }

    return res;
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

int PipeResultSink::fd() const {
    return read_end_.get_fd();
}

void PipeResultSink::close_read_end_from_child() {
    read_end_ = core::FdGuard();
}

}  // namespace kasptest::sys::ipc
