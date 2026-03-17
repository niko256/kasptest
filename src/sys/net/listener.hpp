#pragma once

#include "../../core/fd_guard.hpp"
#include "conn.hpp"

namespace kasptest::sys::net {

/* @brief RAII wrapper for server-side TCP sock */
class TcpListener {
  private:
    core::FdGuard fd_guard_;

  public:
    /* creates listening socket on given port */
    explicit TcpListener(int port);

    /* handles new connection */
    TcpConnection accept();

    int fd() const;
};

}  // namespace kasptest::sys::net
