/// \file Account.hpp
/// \brief Bank accounts: the common base class and the two account types.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bank/AccountObserver.hpp"
#include "bank/AccountState.hpp"
#include "bank/Money.hpp"

namespace bank {

enum class AccountType { Checking, Savings };

/// "checking" or "savings".
std::string toString(AccountType type);

/// \brief Base class of all accounts.
///
/// Holds what every account has (id, owner, balance, state); subclasses only
/// decide how much money is available for a withdrawal (availableFunds()).
/// What is allowed at all (deposit, withdraw, freeze, ...) is decided by the
/// current AccountState, to which every operation is forwarded.
class Account {
 public:
  Account(std::string id, std::string owner);
  virtual ~Account() = default;
  // An account is a unique entity: it must not be copied by accident.
  Account(const Account&) = delete;
  Account& operator=(const Account&) = delete;
  Account(Account&&) = delete;
  Account& operator=(Account&&) = delete;

  const std::string& id() const { return id_; }
  const std::string& owner() const { return owner_; }
  Money balance() const { return balance_; }

  virtual AccountType type() const = 0;
  /// The most that can be withdrawn right now.
  virtual Money availableFunds() const = 0;

  /// Adds money. Throws InvalidAmount if amount <= 0, InvalidOperation if
  /// the state does not allow deposits (closed).
  void deposit(Money amount);
  /// Takes money out. Throws InvalidAmount if amount <= 0, InsufficientFunds
  /// if amount > availableFunds(), InvalidOperation if frozen or closed.
  void withdraw(Money amount);

  /// Blocks withdrawals (e.g. suspected fraud). Only from active.
  void freeze();
  /// Lifts a freeze. Only from frozen.
  void unfreeze();
  /// Closes the account for good. Only with a zero balance.
  void close();

  const AccountState& state() const { return *state_; }
  /// "active", "frozen" or "closed".
  std::string stateName() const { return state_->name(); }

  /// Registers an observer. The account only keeps a weak reference: it does
  /// not keep observers alive, and destroyed observers are skipped.
  void addObserver(const std::shared_ptr<AccountObserver>& observer);
  void removeObserver(const std::shared_ptr<AccountObserver>& observer);
  std::size_t observerCount() const;

 private:
  void notify(EventType type, Money amount, const std::string& detail = {});

  friend class AccountState;  // states change balance_ and state_

  std::string id_;
  std::string owner_;
  Money balance_;
  const AccountState* state_ = &ActiveState::instance();
  std::vector<std::weak_ptr<AccountObserver>> observers_;
};

/// \brief Everyday account that may go below zero, down to an overdraft limit.
class CheckingAccount final : public Account {
 public:
  CheckingAccount(std::string id, std::string owner, Money overdraftLimit);
  AccountType type() const override { return AccountType::Checking; }
  Money availableFunds() const override { return balance() + overdraftLimit_; }
  Money overdraftLimit() const { return overdraftLimit_; }

 private:
  Money overdraftLimit_;
};

/// \brief Account that earns interest and can never go below zero.
class SavingsAccount final : public Account {
 public:
  SavingsAccount(std::string id, std::string owner, double annualInterestPercent);
  AccountType type() const override { return AccountType::Savings; }
  Money availableFunds() const override { return balance(); }
  double annualInterestPercent() const { return annualInterestPercent_; }
  /// Interest for one month on the current balance, rounded to the cent.
  Money monthlyInterest() const;

 private:
  double annualInterestPercent_;
};

}  // namespace bank
