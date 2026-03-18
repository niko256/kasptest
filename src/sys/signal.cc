#include "signal.hpp"
#include <atomic>
#include <csignal>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace kasptest::sys::signal {


std::atomic<bool> running_{true};

void handler(int signal) {
    running_.store(false);
}

void setup_signal_handlers() {
    if (std::signal(SIGINT, handler) == SIG_ERR) {
        /* spdlog::error("signal handler setup for SIGINT failed..."); */
        throw std::runtime_error("failed to setup SIGINT handler...");
    }

    if (std::signal(SIGTERM, handler) == SIG_ERR) {
        /* spdlog::error("signal handler setup for SIGTERM failed...") */
        throw std::runtime_error("failed to setup SIGTERM handler...");
    }

    spdlog::trace("setup for signal handler has been finished...");
}

bool is_running() {
    return running_.load();
}


}  // namespace kasptest::sys::signal
