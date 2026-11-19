#include <gtest/gtest.h>

#include "bank/Version.hpp"

TEST(Version, IsSemanticVersion) { EXPECT_EQ(bank::version(), "1.0.0"); }
