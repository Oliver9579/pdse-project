/// \file Money.hpp
/// \brief Exact money amounts stored as integer cents.
#pragma once

#include <string>

namespace bank {

/// \brief An amount of money, stored as a whole number of cents.
///
/// Money is never stored in a double: 0.1 + 0.2 != 0.3 in floating point,
/// and rounding errors must not appear in account balances. Integer cents
/// make every addition and subtraction exact.
class Money {
 public:
  constexpr Money() = default;

  static constexpr Money fromCents(long long cents) { return Money(cents); }
  /// Parses "12", "12.5", "12.50" or "-3.07"; throws InvalidAmount otherwise.
  static Money parse(const std::string& text);

  constexpr long long cents() const { return cents_; }
  constexpr bool isPositive() const { return cents_ > 0; }
  constexpr bool isNegative() const { return cents_ < 0; }

  /// Formats as "1234.56" or "-0.05".
  std::string toString() const;

  constexpr Money operator+(Money o) const { return Money(cents_ + o.cents_); }
  constexpr Money operator-(Money o) const { return Money(cents_ - o.cents_); }
  constexpr Money operator-() const { return Money(-cents_); }
  Money& operator+=(Money o) {
    cents_ += o.cents_;
    return *this;
  }
  Money& operator-=(Money o) {
    cents_ -= o.cents_;
    return *this;
  }

  constexpr bool operator==(Money o) const { return cents_ == o.cents_; }
  constexpr bool operator!=(Money o) const { return cents_ != o.cents_; }
  constexpr bool operator<(Money o) const { return cents_ < o.cents_; }
  constexpr bool operator<=(Money o) const { return cents_ <= o.cents_; }
  constexpr bool operator>(Money o) const { return cents_ > o.cents_; }
  constexpr bool operator>=(Money o) const { return cents_ >= o.cents_; }

 private:
  constexpr explicit Money(long long cents) : cents_(cents) {}
  long long cents_ = 0;
};

/// Convenience literal-like helper: euros(12, 50) == 12.50.
constexpr Money euros(long long whole, long long cents = 0) {
  return Money::fromCents(whole * 100 + (whole < 0 ? -cents : cents));
}

}  // namespace bank
