#pragma once

#include <string>
#include "../core/entities.hpp"

namespace kasptest::utils::fmt {

/*
 * @brief human-readable summary...
 */

auto format_result(const core::ScanResult& scan) -> std::string;

auto format_stats(const core::StatSnapshot& snapshot) -> std::string;

}  // namespace kasptest::utils::fmt
