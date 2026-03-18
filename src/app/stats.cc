#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include "../core/entities.hpp"
#include "../utils/format.hpp"
#include <sys/ipc.hpp>

using namespace kasptest;

int main() {
    try {
        spdlog::set_level(spdlog::level::info);
        spdlog::info("Requesting statistics from server...");

        sys::ipc::ClientStatsPipeEndpoint client("/tmp/stats");
        core::StatSnapshot stats = client.request_stats();

        std::cout << "\n" << utils::fmt::format_stats(stats);

    } catch (const std::exception& e) {
        spdlog::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}
