#include <gtest/gtest.h>
#include "../src/core/entities.hpp"

/* *---*---*---*---*---*---*---*---*---*---*---*---* */

TEST(StatTest, CleanFile) {
    kasptest::core::StatSnapshot stats;
    kasptest::core::ScanResult clean;

    stats.record(clean);
    EXPECT_EQ(stats.files_checked_, 1);
    EXPECT_TRUE(stats.statistics_.empty());
}

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

/* *---*---*---*---*---*---*---*---*---*---*---*---* */
