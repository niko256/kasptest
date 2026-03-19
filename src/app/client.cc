#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

#include "../core/entities.hpp"
#include "../utils/repr.hpp"
#include "../utils/format.hpp"

#include "../sys/net/conn.hpp"

namespace kasptest {

static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::ostringstream ss;

    spdlog::info("[debug] : trying to read another file... : {}", path);


    ss << file.rdbuf();
    return ss.str();
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static std::pair<std::string, uint16_t> parse_args(int argc, char* argv[]) {
    if (argc != 3) {
        throw std::invalid_argument("Usage: " + std::string(argv[0]) + " file_path port");
    }

    std::string file_path = argv[1];

    int port_int = std::stoi(argv[2]);
    if (port_int < 1 or port_int > 65535) {
        throw std::out_of_range("Port must be in range 1-65535");
    }

    return {file_path, port_int};
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static core::ScanResult send_for_scan(const std::string& file_content, uint16_t port) {
    auto conn = sys::net::TcpConnection::connect("127.0.0.1", port);

    conn.send_all(file_content);
    /*
     * spdlog::info("[debug] : all file content has beeb sent to the server...");
     */
    conn.shutdown_write();

    std::string response = conn.recv_all();
    /*
     * spdlog::info("[debug] : i just got new response from server ^_^");
     */

    return utils::repr::deserialize_result(response);
}

}  // namespace kasptest

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

using namespace kasptest;

int main(int argc, char* argv[]) {
    try {
        auto [file_path, port] = parse_args(argc, argv);

        spdlog::set_level(spdlog::level::info);

        spdlog::info("Reading file: {}", file_path);
        /*
         * ---
         */
        std::string content = read_file(file_path);
        /*
         * ---
         */
        spdlog::info("File size: {} bytes", content.size());

        spdlog::info("Sending to server on port {}...", port);
        core::ScanResult result = send_for_scan(content, port);

        std::cout << "\n" << utils::fmt::format_result(result);

    } catch (const std::exception& e) {
        spdlog::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}
