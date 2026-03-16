#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "../core/entities.hpp"

namespace kasptest::utils::repr {

auto parse_config(std::string_view content) -> std::vector<core::Infection>;

auto serialize_result(const core::ScanResult& res) -> std::string;
auto deserialize_result(std::string_view content) -> core::ScanResult;

auto serialize_stats(const core::StatSnapshot& stats) -> std::string;
auto deserialize_stats(std::string_view content) -> core::StatSnapshot;

}  // namespace kasptest::utils::repr
