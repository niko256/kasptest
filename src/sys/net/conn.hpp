#pragma once

#include "../../core/fd_guard.hpp"
#include <string_view>

namespace kasptest::sys::net {


class TcpConnection {
  private:
    core::FdGuard fd_guard_;

  public:
    explicit TcpConnection(int fd);

    static TcpConnection connect(const std::string& host, int port);


    /*
     * https://stackoverflow.com/questions/67509709/is-recvbufsize-guaranteed-to-receive-all-the-data-if-sended-data-is-smaller-th
     */
    void send_all(std::string_view data);

    std::string recv_all();

    void shutdown_write();

    int fd() const;
};


}  // namespace kasptest::sys::net
