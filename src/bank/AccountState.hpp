/// \file AccountState.hpp
/// \brief The states of an account: Active, Frozen, Closed (State pattern).
#pragma once

#include <string>

#include "bank/Money.hpp"

namespace bank {

class Account;

/// \brief Design pattern: **State**.
///
/// What an account may do depends on its state:
///
/// | operation | Active        | Frozen          | Closed |
/// |-----------|---------------|-----------------|--------|
/// | deposit   | yes           | yes             | no     |
/// | withdraw  | yes           | no              | no     |
/// | freeze    | -> Frozen     | no              | no     |
/// | unfreeze  | no            | -> Active       | no     |
/// | close     | -> Closed (*) | -> Closed (*)   | no     |
///
/// (*) only when the balance is exactly zero.
///
/// Instead of `if (frozen) ... else if (closed) ...` in every Account method,
/// each state is a class and Account forwards the request to its current
/// state object. Adding a new state (e.g. "Dormant") means adding a class,
/// not editing every method (Open-Closed Principle).
///
/// The base class rejects every operation with InvalidOperation; each state
/// overrides only what it allows. States hold no data, so a single shared
/// instance of each is used by all accounts.
class AccountState {
 public:
  AccountState() = default;
  virtual ~AccountState() = default;
  AccountState(const AccountState&) = delete;
  AccountState& operator=(const AccountState&) = delete;
  AccountState(AccountState&&) = delete;
  AccountState& operator=(AccountState&&) = delete;

  virtual std::string name() const = 0;

  virtual void deposit(Account& account, Money amount) const;
  virtual void withdraw(Account& account, Money amount) const;
  virtual void freeze(Account& account) const;
  virtual void unfreeze(Account& account) const;
  virtual void close(Account& account) const;

 protected:
  // Only states may change the balance or the state of an account.
  static void credit(Account& account, Money amount);
  static void debit(Account& account, Money amount);
  static void changeState(Account& account, const AccountState& next);
  /// Throws InvalidOperation("Cannot <operation>: account <id> is <state>").
  [[noreturn]] void reject(const Account& account, const std::string& operation) const;
};

class ActiveState final : public AccountState {
 public:
  static const ActiveState& instance();
  std::string name() const override { return "active"; }
  void deposit(Account& account, Money amount) const override;
  void withdraw(Account& account, Money amount) const override;
  void freeze(Account& account) const override;
  void close(Account& account) const override;
};

class FrozenState final : public AccountState {
 public:
  static const FrozenState& instance();
  std::string name() const override { return "frozen"; }
  void deposit(Account& account, Money amount) const override;  // incoming money still arrives
  void unfreeze(Account& account) const override;
  void close(Account& account) const override;
};

class ClosedState final : public AccountState {
 public:
  static const ClosedState& instance();
  std::string name() const override { return "closed"; }
  // everything is rejected (base class behaviour)
};

}  // namespace bank
