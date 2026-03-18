#include "ipc.hpp"
#include <exception>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include "../utils/repr.hpp"
#include "core/entities.hpp"
#include "sys/fifo_guard.hpp"

namespace kasptest::sys::ipc {

ServerStatsPipeEndpoint::ServerStatsPipeEndpoint(const std::string& base_path)
    : req_fifo_gd_(base_path + "_req"),
      resp_fifo_gd_(base_path + "_resp") {
    /*
     *
     */
    spdlog::info("[debug] : i'm inside server-side pipe endpoint...");
    /*
     *
     */
    req_read_fd_ = req_fifo_gd_.open_for_read();
}

int ServerStatsPipeEndpoint::fd() const {
    return req_read_fd_.get_fd();
}

void ServerStatsPipeEndpoint::handle_request(const core::StatSnapshot& statss) {
    char buff[64];

    while (true) {
        auto bytes_r = read(req_read_fd_.get_fd(), buff, sizeof(buff));
        if (bytes_r <= 0) {
            break;
        }

        spdlog::trace("[trace] : handle_requst(...)");
        spdlog::info("statistics request!");

        std::string resp_json = utils::repr::serialize_stats(statss);

        try {
            auto resp_write_fd = resp_fifo_gd_.open_for_write();

            auto written = 0;
            while (written < resp_json.size()) {
                size_t res = write(
                    resp_write_fd.get_fd(),
                    resp_json.data() + written,
                    resp_json.size() - written
                );

                if (res > 0) {
                    written += res;
                } else if (res < 0) {
                    spdlog::trace("[trace] : handle request");
                    spdlog::warn("failed to write stats response...");
                    break;
                }
            }
        } catch (const std::exception& er) {
            spdlog::error("failed to open resp_fifo for writing... {}", er.what());
        }
    }
}

ClientStatsPipeEndpoint::ClientStatsPipeEndpoint(const std::string& base_path)
    : req_path_(base_path + "_req"),
      resp_path_(base_path + "_resp") {}

auto ClientStatsPipeEndpoint::request_stats() -> core::StatSnapshot {
    {
        FifoGuard req(req_path_);
        auto write_fd = req.open_for_write();

        char ping = '1';
        write(write_fd.get_fd(), &ping, 1);
    }

    FifoGuard resp(resp_path_);
    auto read_fd = resp.open_for_read(true);

    std::string json_response;
    char buf[4096];

    while (true) {
        ssize_t bytes_r = read(read_fd.get_fd(), buf, sizeof(buf));
        if (bytes_r > 0) {
            json_response.append(buf, bytes_r);
        } else if (bytes_r == 0) {
            break;
        } else if (errno != EINTR) {
            throw std::runtime_error("Failed to read from response fifo");
        }
    }

    return utils::repr::deserialize_stats(json_response);
}


}  // namespace kasptest::sys::ipc
