/// \file Bank.hpp
/// \brief The bank: owns the accounts and runs transactions through commands.
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bank/Account.hpp"
#include "bank/AccountFactory.hpp"
#include "bank/AccountObserver.hpp"
#include "bank/Command.hpp"

namespace bank {

/// \brief Simple entry point that ties the patterns together (a Facade).
///
///  * accounts are created by the AccountFactory and owned by the bank,
///  * money operations are wrapped in Commands and run through one
///    CommandHistory, so they can be undone/redone in order,
///  * observers registered on the bank are attached to every account,
///    existing and future.
///
/// Freezing, unfreezing and closing are administrative actions, not money
/// transactions, so they are not undoable.
class Bank {
 public:
  explicit Bank(AccountFactory factory = AccountFactory{});

  /// Opens a new account with the next free id ("ACC-0001", "ACC-0002", ...).
  Account& openAccount(AccountType type, const std::string& owner);

  /// Throws AccountNotFound for an unknown id.
  Account& account(const std::string& id);
  const Account& account(const std::string& id) const;
  std::vector<std::string> accountIds() const;
  /// Sum of all balances.
  Money totalBalance() const;

  void deposit(const std::string& id, Money amount);
  void withdraw(const std::string& id, Money amount);
  void transfer(const std::string& from, const std::string& to, Money amount);
  /// Pays monthly interest; throws InvalidOperation for non-savings accounts.
  Money payInterest(const std::string& id);

  void freeze(const std::string& id) { account(id).freeze(); }
  void unfreeze(const std::string& id) { account(id).unfreeze(); }
  void close(const std::string& id) { account(id).close(); }

  bool undo() { return history_.undo(); }
  bool redo() { return history_.redo(); }
  const CommandHistory& history() const { return history_; }

  /// Attaches the observer to all current and future accounts.
  void addObserver(const std::shared_ptr<AccountObserver>& observer);

 private:
  AccountFactory factory_;
  std::map<std::string, std::unique_ptr<Account>> accounts_;
  CommandHistory history_;
  std::vector<std::shared_ptr<AccountObserver>> observers_;
  int nextNumber_ = 1;
};

}  // namespace bank
