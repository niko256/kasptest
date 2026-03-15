#include <gtest/gtest.h>

class test : public ::testing::Test {
  protected:
    int whatever;
};

TEST_F(test, okay) {
    whatever = 1;
}
