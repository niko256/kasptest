#pragma once

#include "../core/fd_guard.hpp"
#include <string_view>
#include <string>

namespace kasptest::sys::ipc {

/* @brief RAII wrapper for mkfifo pipe  */
class FifoGuard {
  private:
    std::string path_;

  public:
    explicit FifoGuard(const std::string& path);

    ~FifoGuard();

    FifoGuard(const FifoGuard&) = delete;
    FifoGuard& operator=(const FifoGuard&) = delete;

    FifoGuard(FifoGuard&&) noexcept;
    FifoGuard& operator=(FifoGuard&&) noexcept;

    auto path() const -> std::string_view;

    /* @note : maybe we don't wanna block here..? */
    auto open_for_read(bool blocking = false) -> core::FdGuard;
    auto open_for_write() const -> core::FdGuard;
};

}  // namespace kasptest::sys::ipc
