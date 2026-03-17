#pragma once

#include "../core/interfaces.hpp"
#include "../core/fd_guard.hpp"
#include <vector>

namespace kasptest::sys::ipc {

class PipeResultSink : core::IResultSink {
  private:
    core::FdGuard read_end_;
    core::FdGuard write_end_;

    std::string read_buff_;

  public:
    PipeResultSink();

    PipeResultSink(const PipeResultSink&) = delete;
    PipeResultSink(PipeResultSink&&) = delete;
    PipeResultSink& operator=(const PipeResultSink&) = delete;
    PipeResultSink& operator=(PipeResultSink&&) = delete;

    void send(const core::ScanResult& res) override;

    std::vector<core::ScanResult> collect() override;

    int fd() const;

    void close_read_end_from_child();
};

}  // namespace kasptest::sys::ipc
