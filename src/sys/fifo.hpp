#pragma once

#include "../core/fd_guard.hpp"
#include <string_view>
#include <string>

namespace kasptest::sys::ipc {

/* @brief RAII wrapper for mkfifo pipe  */
class Fifo {
  private:
    std::string path_;

  public:
    explicit Fifo(const std::string& path);

    ~Fifo();

    Fifo(const Fifo&) = delete;
    Fifo& operator=(const Fifo&) = delete;

    Fifo(Fifo&&) noexcept;
    Fifo& operator=(Fifo&&) noexcept;

    auto path() const -> std::string_view;

    auto open_for_read() -> core::FdGuard;
    auto open_for_write() const -> core::FdGuard;
};

}  // namespace kasptest::sys::ipc
