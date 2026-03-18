#include "fork_spawn.hpp"
#include <cstdlib>
#include <exception>
#include <functional>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <spdlog/spdlog.h>

namespace kasptest::sys::exec {

ForkSpawner::~ForkSpawner() {
    wait_all();
}

void ForkSpawner::spawn(std::function<void()> task) {
    pid_t pid = fork();

    if (pid < 0) {
        throw std::runtime_error("fork() failed...");
    }

    if (pid == 0) {
        try {
            task();
        } catch (const std::exception& er) {
            spdlog::error("exception in child process... : {}", er.what());
        }

        _exit(0);
    } else {
        spdlog::info("spawned child process with pid:={}", pid);
        children_.push_back(pid);
    }
}

void ForkSpawner::wait_all() {
    for (auto pid : children_) {
        waitpid(pid, nullptr, 0);

        spdlog::info("termination of the child proc with pid:={}", pid);
    }
    children_.clear();
}

}  // namespace kasptest::sys::exec
