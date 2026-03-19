#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <core/entities.hpp>
#include <core/scanner.hpp>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utils/format.hpp>
#include <utils/repr.hpp>
#include <core/scanner.hpp>
#include <vector>

using namespace kasptest;

static std::string test_read_from_config(const std::string& path) {
    std::ifstream f(path);

    if (not f.is_open()) {
        throw std::runtime_error("failed to open config file..." + path);
    }

    std::ostringstream oss;
    oss << f.rdbuf();

    return oss.str();
}

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

class ScannerFixt : public ::testing::Test {
  protected:
    static std::vector<core::Infection> pattrns_;

    static void SetUpTestSuite() {
        auto config_data = test_read_from_config("config.json");
        pattrns_ = utils::repr::parse_config(std::move(config_data));
    }

    core::ScanResult scan_str(const std::string& content) {
        core::StreamScanner scanner(pattrns_);

        scanner.feed(content);
        return scanner.summary();
    }

    core::ScanResult scan_chunked(std::string_view data, size_t ch_sz) {
        core::StreamScanner sc(pattrns_);

        for (int i = 0; i < data.size(); i += ch_sz) {
            size_t len = std::min(ch_sz, data.size() - i);
            sc.feed(std::string_view(data.data() + i, len));
        }

        return sc.summary();
    }
};

/* definition */
std::vector<core::Infection> ScannerFixt::pattrns_;

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

TEST(StatTest, Record) {
    kasptest::core::StatSnapshot stts;
    kasptest::core::ScanResult res1;
    kasptest::core::ScanResult res2;

    res1.threats_ = {{"Trojan", 2}, {"Ugroza", 1}};

    res2.threats_ = {{"Trojan", 3}};

    stts.record(res1);
    stts.record(res2);
    EXPECT_EQ(stts.files_checked_, 2);
    EXPECT_EQ(stts.statistics_.at("Trojan"), 5);
    EXPECT_EQ(stts.statistics_.at("Ugroza"), 1);
}

TEST_F(ScannerFixt, CfgLoad) {
    EXPECT_EQ(pattrns_.size(), 7);
}

TEST_F(ScannerFixt, PureClean) {
    auto res = scan_str("this is a pretty normal safe file w/o any ugrozas");
    EXPECT_TRUE(res.threats_.empty());
}

TEST_F(ScannerFixt, DetectOneInfection) {
    auto res = scan_str(
        "this script will remove french from your system via rm -rf / command"
    );

    ASSERT_EQ(res.threats_.size(), 1);
    EXPECT_EQ(res.threats_[0].pattrn_id_, "Trojan.RemoveFrench");
    EXPECT_EQ(res.threats_[0].counter_, 1);
}

TEST_F(ScannerFixt, DirtyVulnarable) {
    auto res = scan_str("rm -rf / first\nrm -rf / second\nrm -rf / third");
    ASSERT_EQ(res.threats_.size(), 1);
    EXPECT_EQ(res.threats_[0].counter_, 3);
}

TEST_F(ScannerFixt, DetectsMultipleDifferentInfections) {
    auto res = scan_str(":(){ :|:& };: and then rm -rf / and exec(compile( something");
    EXPECT_GE(res.threats_.size(), 3);
}

TEST_F(ScannerFixt, EmptyCleanFile) {
    auto res = scan_str("");
    EXPECT_TRUE(res.threats_.empty());
}

TEST_F(ScannerFixt, DumbActionTest) {
    auto res = scan_str("     \n\n\n\t\t\n\n     ");
    EXPECT_TRUE(res.threats_.empty());
}

TEST_F(ScannerFixt, PatrernSplitBetweenChunks) {
    auto res = scan_chunked("before :(){ :|:& };: after", 7);

    ASSERT_EQ(res.threats_.size(), 1);
    EXPECT_EQ(res.threats_[0].pattrn_id_, "Trojan.forkbomb");
}

TEST_F(ScannerFixt, ByteByByte) {
    auto res = scan_chunked("XXXrm -rf /YYY", 1);

    ASSERT_EQ(res.threats_.size(), 1);
    EXPECT_EQ(res.threats_[0].pattrn_id_, "Trojan.RemoveFrench");
}

TEST_F(ScannerFixt, AtTheEnd) {
    auto res = scan_chunked("safe safe safe rm -rf /", 5);
    ASSERT_EQ(res.threats_.size(), 1);
}

TEST(StatisticsTest, RecordCountsCleanFiles) {
    core::StatSnapshot stats;
    core::ScanResult clean;

    stats.record(clean);
    stats.record(clean);

    EXPECT_EQ(stats.files_checked_, 2);
    EXPECT_TRUE(stats.statistics_.empty());
}

TEST(StatisticsTest, AcrossMultipleFiles) {
    core::StatSnapshot stats;

    core::ScanResult r1;
    r1.threats_ = {{"Trojan.RemoveFrench", 2}, {"Trojan.forkbomb", 1}};

    core::ScanResult r2;
    r2.threats_ = {{"Trojan.RemoveFrench", 3}};

    core::ScanResult r3;

    stats.record(r1);
    stats.record(r2);
    stats.record(r3);

    EXPECT_EQ(stats.files_checked_, 3);
    EXPECT_EQ(stats.statistics_.at("Trojan.RemoveFrench"), 5);
    EXPECT_EQ(stats.statistics_.at("Trojan.forkbomb"), 1);
}

TEST(ProtoTest, ScanResultSpin) {
    core::ScanResult original;
    original.threats_ = {{"Trojan.RemoveFrench", 5}, {"Trojan.forkbomb", 2}};
    std::string json = utils::repr::serialize_result(original);
    auto restored = utils::repr::deserialize_result(json);

    ASSERT_EQ(restored.threats_.size(), 2);
    EXPECT_EQ(restored.threats_[0].pattrn_id_, "Trojan.RemoveFrench");


    EXPECT_EQ(restored.threats_[0].counter_, 5);
    EXPECT_EQ(restored.threats_[1].pattrn_id_, "Trojan.forkbomb");
    EXPECT_EQ(restored.threats_[1].counter_, 2);
}

TEST(ProtoTest, StatisticsSpin) {
    core::StatSnapshot original;
    original.files_checked_ = 42;
    original.statistics_ = {{"Trojan.RemoveFrench", 15}, {"Trojan.forkbomb", 3}};

    std::string json = utils::repr::serialize_stats(original);
    auto restored = utils::repr::deserialize_stats(json);

    EXPECT_EQ(restored.files_checked_, 42);
    EXPECT_EQ(restored.statistics_.at("Trojan.RemoveFrench"), 15);
    EXPECT_EQ(restored.statistics_.at("Trojan.forkbomb"), 3);
}

TEST_F(ScannerFixt, FullPipelineInfectedFile) {
    auto result = scan_str("this does rm -rf / and :(){ :|:& };: too");

    std::string json = utils::repr::serialize_result(result);

    auto rest = utils::repr::deserialize_result(json);

    ASSERT_EQ(rest.threats_.size(), 2);

    std::string output = utils::fmt::format_result(rest);

    EXPECT_NE(output.find("Trojan.RemoveFrench"), std::string::npos);

    EXPECT_NE(output.find("Trojan.forkbomb"), std::string::npos);
}

TEST_F(ScannerFixt, FullPipelineCleanFile) {
    auto result = scan_str("nothing dangerous here at all");


    std::string json = utils::repr::serialize_result(result);
    auto rest = utils::repr::deserialize_result(json);
    std::string output = utils::fmt::format_result(rest);

    EXPECT_TRUE(rest.threats_.empty());
    EXPECT_NE(output.find("CLEAN"), std::string::npos);
}
