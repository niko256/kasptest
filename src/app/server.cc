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
    std::vector<core::Infection> patterns_;
    core::StatSnapshot statistics_;
    sys::net::TcpListener listener_;
    sys::ipc::PipeResultSink sink_;
    sys::exec::ForkSpawner spawner_;
    sys::ipc::ServerStatsPipeEndpoint stats_endpoint_;
    sys::event::Poller poller_;

    /* ---*---*---*---*---*---*---*---*---*---*---*---* */

    ServerContext(std::vector<core::Infection> p, uint16_t port)
        : patterns_(std::move(p)),
          listener_(port),
          stats_endpoint_("/tmp/stats") {
        /*
         * ---
         */
        spdlog::info("[debug] : listener created on fd := {}", listener_.fd());
        spdlog::info("[debug] : sink created on fd := {}", sink_.fd());
        spdlog::info(
            "[debug] : stats endpoind created on fd := {}", stats_endpoint_.fd()
        );
        /*
         * ---
         */

        poller_.subscribe(listener_.fd());
        poller_.subscribe(sink_.fd());
        poller_.subscribe(stats_endpoint_.fd());

        spdlog::info("[debug] : poller has been subscribed on all fds");
    }
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static std::string read_file_to_str(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::ostringstream ss;

    ss << file.rdbuf();
    return ss.str();
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

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

    return {config_path, port_int};
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static void handle_single_client(
    sys::net::TcpConnection conn,
    const std::vector<core::Infection>& patterns,
    sys::ipc::PipeResultSink& sink
) {
    sink.close_read_end_from_child();


    spdlog::info("[child]: receiving file...");
    std::string file_content = conn.recv_all();
    spdlog::info("[child]: received {} bytes", file_content.size());

    core::StreamScanner scanner(patterns);
    scanner.feed(file_content);
    core::ScanResult result = scanner.summary();

    spdlog::info(
        "Child: infected={}, threats={}", !result.threats_.empty(), result.threats_.size()
    );

    conn.send_all(utils::repr::serialize_result(result));

    sink.send(result);
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static void accept_new_client(ServerContext& ctx) {
    try {
        sys::net::TcpConnection conn = ctx.listener_.accept();

        int client_fd = conn.release();

        ctx.spawner_.spawn([client_fd, &ctx]() mutable {
            sys::net::TcpConnection child_conn(client_fd);
            handle_single_client(std::move(child_conn), ctx.patterns_, ctx.sink_);
        });
    } catch (const std::exception& e) {
        spdlog::error("Failed to accept client: {}", e.what());
    }
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static void collect_results(ServerContext& ctx) {
    for (const auto& result : ctx.sink_.collect()) {
        ctx.statistics_.record(result);
        spdlog::info("Stats updated: {} files checked", ctx.statistics_.files_checked_);
    }
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static void process_events(ServerContext& ctx) {
    auto events = ctx.poller_.wait(1000);

    for (const auto& event : events) {
        if (event.fd_ == ctx.listener_.fd()) {
            accept_new_client(ctx);
        } else if (event.fd_ == ctx.stats_endpoint_.fd()) {
            ctx.stats_endpoint_.handle_request(ctx.statistics_);
        }
    }

    collect_results(ctx);

    /* spdlog::info("[debug] : all results from events has been looted ^_^"); */
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

static void shutdown_server(ServerContext& ctx) {
    spdlog::info("Shutting down...");

    ctx.poller_.unsubscribe(ctx.listener_.fd());

    ctx.spawner_.wait_all();

    collect_results(ctx);

    spdlog::info(
        "Server stopped. Total files checked: {}", ctx.statistics_.files_checked_
    );
}

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

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


        spdlog::info("[debug] : server is shutting down...");
        shutdown_server(ctx);


    } catch (const std::exception& e) {
        spdlog::error("Fatal: {}", e.what());
        spdlog::default_logger()->flush();

        return 1;
    }

    return 0;
}
