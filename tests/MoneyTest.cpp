#include <gtest/gtest.h>

#include "bank/Errors.hpp"
#include "bank/Money.hpp"

using bank::euros;
using bank::InvalidAmount;
using bank::Money;

TEST(Money, ArithmeticIsExact) {
  // the classic floating point trap: 0.1 + 0.2 must be exactly 0.3
  EXPECT_EQ(Money::parse("0.10") + Money::parse("0.20"), Money::parse("0.30"));
  Money m = euros(10);
  m += euros(2, 50);
  m -= euros(0, 75);
  EXPECT_EQ(m.cents(), 1175);
  EXPECT_EQ(-m, Money::fromCents(-1175));
  EXPECT_EQ(euros(-3, 7).cents(), -307);
}

TEST(Money, Comparisons) {
  EXPECT_LT(euros(1), euros(2));
  EXPECT_GE(euros(2), euros(2));
  EXPECT_NE(euros(1), euros(1, 1));
  EXPECT_TRUE(euros(1).isPositive());
  EXPECT_TRUE(euros(-1).isNegative());
  EXPECT_FALSE(Money().isPositive());
}

TEST(Money, ToString) {
  EXPECT_EQ(euros(1234, 56).toString(), "1234.56");
  EXPECT_EQ(Money::fromCents(5).toString(), "0.05");
  EXPECT_EQ(Money::fromCents(-5).toString(), "-0.05");
  EXPECT_EQ(Money().toString(), "0.00");
}

TEST(Money, ParseValidInput) {
  EXPECT_EQ(Money::parse("12"), euros(12));
  EXPECT_EQ(Money::parse("12.5"), euros(12, 50));
  EXPECT_EQ(Money::parse("12.05"), euros(12, 5));
  EXPECT_EQ(Money::parse("-3.07"), euros(-3, 7));
  EXPECT_EQ(Money::parse("+1.00"), euros(1));
}

TEST(Money, ParseRejectsInvalidInput) {
  for (const char* bad : {"", "-", "abc", "1.", ".5", "1.234", "1,50", "1.5x", "12 ", "99999999999999"}) {
    EXPECT_THROW(Money::parse(bad), InvalidAmount) << bad;
  }
}
