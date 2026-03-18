#pragma once

#include "core/entities.hpp"
#include "core/fd_guard.hpp"
#include "fifo_guard.hpp"
#include "sys/fifo_guard.hpp"

namespace kasptest::sys::ipc {


class ServerStatsPipeEndpoint {
  private:
    FifoGuard req_fifo_gd_;
    FifoGuard resp_fifo_gd_;
    core::FdGuard req_read_fd_;

  public:
    ServerStatsPipeEndpoint(const std::string& base_path);

    void handle_request(const core::StatSnapshot& statss);

    int fd() const;
};

class ClientStatsPipeEndpoint {
  private:
    std::string req_path_;
    std::string resp_path_;

  public:
    ClientStatsPipeEndpoint(const std::string& path);

    auto request_stats() -> core::StatSnapshot;
};

}  // namespace kasptest::sys::ipc
