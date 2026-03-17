#pragma once

#include "listener.hpp"
#include "conn.hpp"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace kasptest::sys::net {

TcpListener::TcpListener(int port) {
    int scfd = socket(AF_INET, SOCK_STREAM, 0);

    /*   */
    if (scfd < 0) {
        throw std::runtime_error("socket() failed...");
    }

    fd_guard_ = core::FdGuard(scfd);

    int opt = 1; /* switch on */
    /* setting up a socket */
    if (setsockopt(fd_guard_.get_fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed...");
    }

    sockaddr_in addrr{};
    addrr.sin_family = AF_INET;
    addrr.sin_addr.s_addr = INADDR_ANY; /* 0.0.0.0 */
    addrr.sin_port = htons(port);

    if (bind(fd_guard_.get_fd(), reinterpret_cast<sockaddr*>(&addrr), sizeof(addrr))) {
        throw std::runtime_error("bind() failed on port " + std::to_string(port));
    }

    /* btw, SOMAXCONN is the size of the backlog queue for new connections,
     *  that haven't been accepted yet
     */
    if (listen(fd_guard_.get_fd(), SOMAXCONN) < 0) {
        throw std::runtime_error("listen() failed...");
    }

    /* spdlog::info("TcpListener started on port {}, fd:={}", port, fd_.get()); */
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

TcpConnection TcpListener::accept() {
    sockaddr_in cleint_addr{};
    socklen_t clintlen = sizeof(cleint_addr);

    int new_client_fd
        = ::accept(fd_guard_.get_fd(), reinterpret_cast<sockaddr*>(&cleint_addr), &clintlen);

    if (new_client_fd < 0) {
        throw std::runtime_error("accept() failed...");
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cleint_addr.sin_addr, ip_str, INET6_ADDRSTRLEN);

    spdlog::info(
        "new connection from {}:{}, client_fd:={}",
        ip_str,
        ntohs(cleint_addr.sin_port),
        new_client_fd
    );
    return TcpConnection(new_client_fd);
}

int TcpListener::fd() const {
    return fd_guard_.get_fd();
}

}  // namespace kasptest::sys::net
