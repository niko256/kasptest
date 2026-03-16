#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace kasptest::core {

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

struct Infection {
    /* human-readable identifier */
    std::string identifier_;

    /* the exact sequence we are looking for */
    std::string pattern_;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

struct ThreatEntry {
    std::string pattrn_id_;

    int counter_{0};
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

/* @brief represents the threats found during the scan of a single file */
struct ScanResult {
    std::vector<ThreatEntry> threats_;

    /* bool infected_{false}; */
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

/* @brief multiplexed statistics above all the scanned files during current session */
struct StatSnapshot {
    int files_checked_{0};

    /* pattetn_id -> how many times does it appear in the text */
    std::unordered_map<std::string, int> statistics_;

    /* update  */
    void record(const ScanResult& res) {
        ++files_checked_;

        for (auto& entry : res.threats_) {
            statistics_[entry.pattrn_id_] += entry.counter_;
        }
    }
};

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

}  // namespace kasptest::core
