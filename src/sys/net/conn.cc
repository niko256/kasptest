#include "conn.hpp"
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

namespace kasptest::sys::net {

/*
 * this piece of art was very helpful to me ^_^ :
 * https://beej.us/guide/bgnet/translations/bgnet_A4_rus.pdf
 */

TcpConnection::TcpConnection(int fd)
    : fd_(fd) {}

TcpConnection TcpConnection::connect(const std::string& host, int port) {
    core ::FdGuard guard(::socket(AF_INET, SOCK_STREAM, 0));

    if (not guard.valid()) {
        throw std::runtime_error("socket() failed...");
    }

    sockaddr_in addrr{};
    addrr.sin_family = AF_INET;   /* v4 family */
    addrr.sin_port = htons(port); /* little endian -> big endian */

    /* host string -> binary repr */
    if (::inet_pton(AF_INET, host.c_str(), &addrr.sin_addr) <= 0) {
        throw std::runtime_error("invalid address... : " + host);
    }

    if (::connect(guard.get(), reinterpret_cast<sockaddr*>(&addrr), sizeof(addrr)) < 0) {
        throw std::runtime_error("connect() failed...");
    };

    /*
     * spdlog::info("Connection to {}:{} fd={}", host, port, guard.get());
     */

    return TcpConnection(guard.release());
}

void TcpConnection::send_all(std::string_view data) {
    size_t total = 0;

    while (total < data.size()) {
        auto sent = send(fd_.get(), data.data() + total, data.size() - total, MSG_NOSIGNAL);
        /* disable signaling on broken connection */

        if (sent < 0) {
            throw std::runtime_error("send() failed...");
        }

        total += sent;
    }

    /*
     * spdlog::debug("sent {} bytes on fd:={}", data.size(), fd_.get());
     */
}

std::string TcpConnection::recv_all() {
    std::string res;
    char buffer[4096];

    while (true) {
        auto received = recv(fd_.get(), buffer, sizeof(buffer), 0);

        if (received < 0) {
            throw std::runtime_error("recv() failed...");
        } else if (received == 0) {
            break;
        }
        res.append(buffer, received);
    }

    /*
     * spdlog::debug("received {} bytes on fd:={}", res.size(), fd_.get());
     */
    return res;
}

void TcpConnection::shutdown_write() {
    if (shutdown(fd_.get(), SHUT_WR) < 0) {
        throw std::runtime_error("shutdown() failed...");
    }
    /*
     * spdlog::debug("shutdown write on fd:={}", fd_.get());
     */
}

int TcpConnection::fd() const {
    return fd_.get();
}

}  // namespace kasptest::sys::net
