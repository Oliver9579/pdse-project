#include "bank/Money.hpp"

#include <cctype>
#include <cstdlib>

#include "bank/Errors.hpp"

namespace bank {

Money Money::parse(const std::string& text) {
  const auto fail = [&text]() { return InvalidAmount("Invalid amount: '" + text + "'"); };
  std::size_t i = 0;
  bool negative = false;
  if (i < text.size() && (text[i] == '-' || text[i] == '+')) {
    negative = text[i] == '-';
    ++i;
  }
  long long whole = 0;
  std::size_t digits = 0;
  for (; i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])); ++i, ++digits) {
    whole = whole * 10 + (text[i] - '0');
    if (whole > 1'000'000'000'000LL) {
      throw fail();
    }
  }
  if (digits == 0) {
    throw fail();
  }
  long long cents = 0;
  if (i < text.size() && text[i] == '.') {
    ++i;
    std::size_t decimals = 0;
    for (; i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])); ++i) {
      if (++decimals > 2) {
        throw fail();  // fractions of a cent are not allowed
      }
      cents = cents * 10 + (text[i] - '0');
    }
    if (decimals == 0) {
      throw fail();
    }
    if (decimals == 1) {
      cents *= 10;
    }
  }
  if (i != text.size()) {
    throw fail();
  }
  const long long total = whole * 100 + cents;
  return Money(negative ? -total : total);
}

std::string Money::toString() const {
  const long long a = std::llabs(cents_);
  std::string cents = std::to_string(a % 100);
  if (cents.size() < 2) {
    cents.insert(0, "0");
  }
  return std::string(cents_ < 0 ? "-" : "") + std::to_string(a / 100) + "." + cents;
}

}  // namespace bank
