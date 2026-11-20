#include "bank/Account.hpp"

#include <cmath>
#include <utility>

#include "bank/Errors.hpp"

namespace bank {

std::string toString(AccountType type) {
  return type == AccountType::Checking ? "checking" : "savings";
}

Account::Account(std::string id, std::string owner) : id_(std::move(id)), owner_(std::move(owner)) {
  if (id_.empty() || owner_.empty()) {
    throw BankError("An account needs an id and an owner");
  }
}

void Account::deposit(Money amount) {
  if (!amount.isPositive()) {
    throw InvalidAmount("Deposit amount must be positive, got " + amount.toString());
  }
  balance_ += amount;
}

void Account::withdraw(Money amount) {
  if (!amount.isPositive()) {
    throw InvalidAmount("Withdrawal amount must be positive, got " + amount.toString());
  }
  if (amount > availableFunds()) {
    throw InsufficientFunds("Cannot withdraw " + amount.toString() + " from " + id_ +
                            ": only " + availableFunds().toString() + " available");
  }
  balance_ -= amount;
}

CheckingAccount::CheckingAccount(std::string id, std::string owner, Money overdraftLimit)
    : Account(std::move(id), std::move(owner)), overdraftLimit_(overdraftLimit) {
  if (overdraftLimit_.isNegative()) {
    throw InvalidAmount("Overdraft limit cannot be negative");
  }
}

SavingsAccount::SavingsAccount(std::string id, std::string owner, double annualInterestPercent)
    : Account(std::move(id), std::move(owner)), annualInterestPercent_(annualInterestPercent) {
  if (!(annualInterestPercent_ >= 0.0) || annualInterestPercent_ > 100.0) {
    throw BankError("Interest rate must be between 0 and 100 percent");
  }
}

Money SavingsAccount::monthlyInterest() const {
  if (!balance().isPositive()) {
    return Money();
  }
  const double cents =
      static_cast<double>(balance().cents()) * annualInterestPercent_ / 100.0 / 12.0;
  return Money::fromCents(std::llround(cents));
}

}  // namespace bank
