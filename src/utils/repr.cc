#include "repr.hpp"
#include "core/entities.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using json = nlohmann::json;

namespace kasptest::core {


/*
 * https://json.nlohmann.me/api/adl_serializer/from_json/
 */
void from_json(const json& src, Infection& inf) {
    src.at("identifier_").get_to(inf.identifier_);
    src.at("pattern_").get_to(inf.pattern_);
}

void from_json(const json& src, ThreatEntry& entry) {
    src.at("pattrn_id_").get_to(entry.pattrn_id_);
    src.at("counter_").get_to(entry.counter_);
}

void from_json(const json& src, ScanResult& res) {
    src.at("threats_").get_to(res.threats_);
}

void from_json(const json& src, StatSnapshot& snap) {
    src.at("files_checked_").get_to(snap.files_checked_);
    src.at("statistics_").get_to(snap.statistics_);
}

void to_json(json& dest, const Infection& inf) {
    dest = json{{"identifier_", inf.identifier_}, {"pattern_", inf.pattern_}};
}

void to_json(json& dest, const ThreatEntry& entry) {
    dest = json{{"counter_", entry.counter_}, {"pattrn_id_", entry.pattrn_id_}};
}

void to_json(json& dest, const ScanResult& res) {
    dest = json{{"threats_", res.threats_}};
}

void to_json(json& dest, const StatSnapshot& snap) {
    dest = json{
        {"files_checked_", snap.files_checked_}, {"statistics_", snap.statistics_}
    };
}

}  // namespace kasptest::core

namespace kasptest::utils::repr {

auto parse_config(std::string_view content) -> std::vector<core::Infection> {
    try {
        json j = json::parse(content);


        return j.get<std::vector<core::Infection>>();

    } catch (const json::exception& err) { /* https://json.nlohmann.me/home/exceptions/ */
        throw std::invalid_argument(
            "failed to parse config : " + std::string(err.what())
        );
    }
}

auto serialize_result(const core::ScanResult& res) -> std::string {
    json j = res;

    /* json -> str representation */
    return j.dump();
}

auto serialize_stats(const core::StatSnapshot& stats) -> std::string {
    json j = stats;

    return j.dump();
}

auto deserialize_result(std::string_view content) -> core::ScanResult {
    try {
        json j = json::parse(content);

        return j.get<core::ScanResult>();
    } catch (const json::exception& err) {
        throw std::invalid_argument(
            "failed to dererealize scan reuslt : " + std::string(err.what())
        );
    }
}

auto deserialize_stats(std::string_view content) -> core::StatSnapshot {
    try {
        json j = json::parse(content);
        return j.get<core::StatSnapshot>();

    } catch (const json::exception& err) {
        throw std::invalid_argument(
            "failed to deserialize statistics into snapshot : " + std::string(err.what())
        );
    }
}

}  // namespace kasptest::utils::repr
