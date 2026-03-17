#include "format.hpp"
#include <sstream>

namespace kasptest::utils::fmt {


auto format_result(const core::ScanResult& scan) -> std::string {
    std::ostringstream oss;

    oss << "*---*---* FILE SCAN RESULT *---*---*\n";

    if (scan.threats_.empty()) {
        oss << "[status] : CLEAN! \n";
        oss << "=> no threats detected. \n";
        return oss.str();
    }

    oss << "[status] : INFECTED! \n";
    oss << "Threats found : \n";

    for (const auto& thr : scan.threats_) {
        oss << " ->>> " << thr.pattrn_id_ << "[occurrences] : " << thr.counter_ << "\n";
    }
    return oss.str();
}

auto format_stats(const core::StatSnapshot& snapshot) -> std::string {
    std::ostringstream oss;

    oss << "*---*---* STATISTICS *---*---*\n";

    oss << "Total files checked : " << snapshot.files_checked_ << "\n";

    if (snapshot.statistics_.empty()) {
        oss << "[status] : " << "no threats detected so far...\n";
    }

    oss << "Threats detected : \n";
    for (const auto& [pattern_id, count] : snapshot.statistics_) {
        oss << " ->>> " << pattern_id << " : " << count << "\n";
    }
    return oss.str();
}

}  // namespace kasptest::utils::fmt
