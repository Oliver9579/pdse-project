#include "bank/Account.hpp"

#include <algorithm>
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
  state_->deposit(*this, amount);
  notify(EventType::Deposit, amount);
}

void Account::withdraw(Money amount) {
  if (!amount.isPositive()) {
    throw InvalidAmount("Withdrawal amount must be positive, got " + amount.toString());
  }
  state_->withdraw(*this, amount);
  notify(EventType::Withdrawal, amount);
}

void Account::freeze() {
  const auto before = stateName();
  state_->freeze(*this);
  notify(EventType::StateChanged, Money(), before + " -> " + stateName());
}

void Account::unfreeze() {
  const auto before = stateName();
  state_->unfreeze(*this);
  notify(EventType::StateChanged, Money(), before + " -> " + stateName());
}

void Account::close() {
  const auto before = stateName();
  state_->close(*this);
  notify(EventType::StateChanged, Money(), before + " -> " + stateName());
}

void Account::addObserver(const std::shared_ptr<AccountObserver>& observer) {
  if (observer) {
    observers_.push_back(observer);
  }
}

void Account::removeObserver(const std::shared_ptr<AccountObserver>& observer) {
  observers_.erase(std::remove_if(observers_.begin(), observers_.end(),
                                  [&observer](const std::weak_ptr<AccountObserver>& w) {
                                    const auto o = w.lock();
                                    return !o || o == observer;
                                  }),
                   observers_.end());
}

std::size_t Account::observerCount() const {
  return static_cast<std::size_t>(
      std::count_if(observers_.begin(), observers_.end(),
                    [](const std::weak_ptr<AccountObserver>& w) { return !w.expired(); }));
}

void Account::notify(EventType type, Money amount, const std::string& detail) {
  const AccountEvent event{type, id_, amount, balance_, detail};
  // copy first: an observer may add/remove observers while being notified
  const auto observers = observers_;
  for (const auto& weak : observers) {
    if (const auto observer = weak.lock()) {
      observer->onEvent(event);
    }
  }
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
