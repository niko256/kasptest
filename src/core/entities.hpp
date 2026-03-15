#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace kasptest::core {

struct ThreatPattern {
    /* human-readable identifier */
    std::string identifier_;

    std::string pattern_;
};

struct ThreatEntry {
    std::string pattrn_id_;

    int counter{0};
};

struct ScanResult {
    std::vector<ThreatEntry> threats_;

    bool infected_;
};

struct StatSnapshot {
    int files_checked_{0};

    /* pattetn_id -> how many times does it appear in the text */
    std::unordered_map<std::string, int> statistics_;
};

}  // namespace kasptest::core
