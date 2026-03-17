#pragma once

#include <functional>
#include <sys/types.h>
#include <vector>
#include "../core/interfaces.hpp"

namespace kasptest::sys::exec {

class ForkSpawner : public core::ISpawner {
  private:
    std::vector<pid_t> children_;

  public:
    ForkSpawner() = default;
    ~ForkSpawner() override;

    void spawn(std::function<void()> task) override;

    void wait_all() override;
};

}  // namespace kasptest::sys::exec
