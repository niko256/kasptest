#include <fstream>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include "../core/entities.hpp"
#include "../sys/poller.hpp"
#include "../sys/fork_spawn.hpp"
#include "../sys/net/conn.hpp"
#include "../sys/net/listener.hpp"
#include "../sys/pipe_sink.hpp"
#include "../core/scanner.hpp"
#include "../utils/repr.hpp"
#include "../sys/signal.hpp"
#include "spdlog/common.h"
#include "sys/ipc.hpp"

using namespace kasptest;

/* i just invented my own io_context... omg.. */
struct ServerContext {
    std::vector<core::Infection> patterns;
    core::StatSnapshot statistics;
    sys::net::TcpListener listener;
    sys::ipc::PipeResultSink sink;
    sys::exec::ForkSpawner spawner;
    sys::ipc::ServerStatsPipeEndpoint stats_endpoint;
    sys::event::Poller poller;

    ServerContext(std::vector<core::Infection> p, uint16_t port)
        : patterns(std::move(p)),
          listener(port),
          stats_endpoint("/tmp/stats") {
        /*
         * ---
         */
        spdlog::info("[debug] : listener created on fd := {}", listener.fd());
        spdlog::info("[debug] : sink created on fd := {}", sink.fd());
        spdlog::info("[debug] : stats endpoind created on fd := {}", stats_endpoint.fd());
        /*
         * ---
         */

        poller.subscribe(listener.fd());
        poller.subscribe(sink.fd());
        poller.subscribe(stats_endpoint.fd());

        spdlog::info("[debug] : poller has been subscribed on all fds");
    }
};

static std::string read_file_to_str(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::pair<std::string, uint16_t> parse_args(int argc, char* argv[]) {
    if (argc != 3) {
        throw std::invalid_argument(
            "Usage: " + std::string(argv[0]) + " <config.json> <port>"
        );
    }

    std::string config_path = argv[1];

    int port_int = std::stoi(argv[2]);
    if (port_int < 1 || port_int > 65535) {
        throw std::out_of_range("Port must be in range 1-65535");
    }

    return {config_path, static_cast<uint16_t>(port_int)};
}

static void handle_single_client(
    sys::net::TcpConnection conn,
    const std::vector<core::Infection>& patterns,
    sys::ipc::PipeResultSink& sink
) {
    sink.close_read_end_from_child();

    spdlog::info("Worker: receiving file...");
    std::string file_content = conn.recv_all();
    spdlog::info("Worker: received {} bytes", file_content.size());

    core::StreamScanner scanner(patterns);
    scanner.feed(file_content);
    core::ScanResult result = scanner.summary();

    spdlog::info(
        "Worker: infected={}, threats={}", result.threats_.empty(), result.threats_.size()
    );

    conn.send_all(utils::repr::serialize_result(result));

    sink.send(result);
}

static void accept_new_client(ServerContext& ctx) {
    try {
        sys::net::TcpConnection conn = ctx.listener.accept();

        int client_fd = conn.release();

        ctx.spawner.spawn([client_fd, &ctx]() mutable {
            sys::net::TcpConnection child_conn(client_fd);
            handle_single_client(std::move(child_conn), ctx.patterns, ctx.sink);
        });
    } catch (const std::exception& e) {
        spdlog::error("Failed to accept client: {}", e.what());
    }
}

static void collect_results(ServerContext& ctx) {
    for (const auto& result : ctx.sink.collect()) {
        ctx.statistics.record(result);
        spdlog::info("Stats updated: {} files checked", ctx.statistics.files_checked_);
    }
}

static void process_events(ServerContext& ctx) {
    auto events = ctx.poller.wait(1000);

    for (const auto& event : events) {
        if (event.fd_ == ctx.listener.fd()) {
            accept_new_client(ctx);
        } else if (event.fd_ == ctx.stats_endpoint.fd()) {
            ctx.stats_endpoint.handle_request(ctx.statistics);
        }
    }

    collect_results(ctx);
}

static void shutdown_server(ServerContext& ctx) {
    spdlog::info("Shutting down...");

    ctx.poller.unsubscribe(ctx.listener.fd());

    ctx.spawner.wait_all();

    collect_results(ctx);

    spdlog::info(
        "Server stopped. Total files checked: {}", ctx.statistics.files_checked_
    );
}

int main(int argc, char* argv[]) {
    try {
        auto [config_path, port] = parse_args(argc, argv);

        spdlog::set_level(spdlog::level::trace);
        sys::signal::setup_signal_handlers();

        /* config loading... */
        spdlog::info("Loading config from {}", config_path);
        auto patterns = utils::repr::parse_config(read_file_to_str(config_path));
        spdlog::info("Loaded {} patterns", patterns.size());

        spdlog::info("[debug] : i'm alive before server creation...");
        /*
         *
         */
        ServerContext ctx(std::move(patterns), port);
        /*
         *
         */
        spdlog::info("[debug] : i'm alive after server setup...");
        spdlog::info("Server listening on port {}", port);

        while (sys::signal::is_running()) {
            process_events(ctx);
        }

        shutdown_server(ctx);

    } catch (const std::exception& e) {
        spdlog::error("Fatal: {}", e.what());
        spdlog::default_logger()->flush();

        return 1;
    }

    return 0;
}
