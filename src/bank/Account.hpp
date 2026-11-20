/// \file Account.hpp
/// \brief Bank accounts: the common base class and the two account types.
#pragma once

#include <string>

#include "bank/Money.hpp"

namespace bank {

enum class AccountType { Checking, Savings };

/// "checking" or "savings".
std::string toString(AccountType type);

/// \brief Base class of all accounts.
///
/// Holds what every account has (id, owner, balance) and the rules shared by
/// all of them; subclasses only decide how much money is available for a
/// withdrawal (availableFunds()).
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

  /// Adds money. Throws InvalidAmount if amount <= 0.
  void deposit(Money amount);
  /// Takes money out. Throws InvalidAmount if amount <= 0 and
  /// InsufficientFunds if amount > availableFunds().
  void withdraw(Money amount);

 private:
  std::string id_;
  std::string owner_;
  Money balance_;
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
