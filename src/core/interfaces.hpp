#pragma once

#include "entities.hpp"
#include <functional>
#include <vector>

namespace kasptest::core {

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

class ISpawner {
  public:
    virtual ~ISpawner() = default;

    virtual void spawn(std::function<void()> task) = 0;
    virtual void wait_all() = 0;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

class IResultSink {
  public:
    virtual ~IResultSink() = default;

    virtual void send(const ScanResult& res) = 0;

    virtual std::vector<ScanResult> collect() = 0;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

}  // namespace kasptest::core
