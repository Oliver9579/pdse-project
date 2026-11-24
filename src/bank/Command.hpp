/// \file Command.hpp
/// \brief Bank transactions as objects with undo/redo (Command pattern).
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bank/Account.hpp"
#include "bank/Money.hpp"

namespace bank {

/// \brief Design pattern: **Command**.
///
/// Every transaction is an object that knows how to execute() itself and how
/// to undo() it. Because a transaction is an object, it can be stored in a
/// history, undone, redone, logged or queued - none of which is possible with
/// a plain `account.withdraw(x)` call.
///
/// Undo is done with a *reversing* operation through the normal Account API
/// (undoing a deposit is a withdrawal), the way real banks post a reversal
/// instead of erasing history. So all rules still apply: e.g. undoing a
/// deposit on a frozen account is rejected.
class Command {
 public:
  Command() = default;
  virtual ~Command() = default;
  Command(const Command&) = delete;
  Command& operator=(const Command&) = delete;
  Command(Command&&) = delete;
  Command& operator=(Command&&) = delete;

  virtual void execute() = 0;
  virtual void undo() = 0;
  /// Human readable description, e.g. "deposit 100.00 to ACC-0001".
  virtual std::string describe() const = 0;
};

class DepositCommand final : public Command {
 public:
  DepositCommand(Account& account, Money amount) : account_(account), amount_(amount) {}
  void execute() override { account_.deposit(amount_); }
  void undo() override { account_.withdraw(amount_); }
  std::string describe() const override;

 private:
  Account& account_;
  Money amount_;
};

class WithdrawCommand final : public Command {
 public:
  WithdrawCommand(Account& account, Money amount) : account_(account), amount_(amount) {}
  void execute() override { account_.withdraw(amount_); }
  void undo() override { account_.deposit(amount_); }
  std::string describe() const override;

 private:
  Account& account_;
  Money amount_;
};

/// \brief Moves money between two accounts, all or nothing: if the deposit
///        fails, the withdrawal is rolled back.
class TransferCommand final : public Command {
 public:
  /// Throws InvalidOperation if \p from and \p to are the same account.
  TransferCommand(Account& from, Account& to, Money amount);
  void execute() override;
  void undo() override;
  std::string describe() const override;

 private:
  Account& from_;
  Account& to_;
  Money amount_;
};

/// \brief Pays one month of interest into a savings account. The amount is
///        computed when first executed and remembered for undo/redo.
class InterestCommand final : public Command {
 public:
  explicit InterestCommand(SavingsAccount& account) : account_(account) {}
  void execute() override;
  void undo() override;
  std::string describe() const override;
  Money amount() const { return amount_; }

 private:
  SavingsAccount& account_;
  Money amount_;
  bool computed_ = false;
};

/// \brief The *invoker*: executes commands and keeps undo/redo stacks.
class CommandHistory {
 public:
  /// Executes the command; if it succeeds it is put on the undo stack and the
  /// redo stack is cleared. If it throws, nothing is recorded.
  void execute(std::unique_ptr<Command> command);
  /// Undoes the latest command. Returns false if there is nothing to undo.
  /// If the undo itself throws, the history is left unchanged.
  bool undo();
  /// Re-executes the latest undone command. Returns false if there is none.
  bool redo();

  bool canUndo() const { return !undo_.empty(); }
  bool canRedo() const { return !redo_.empty(); }
  /// Descriptions of the undoable commands, most recent first.
  std::vector<std::string> undoList() const;

 private:
  std::vector<std::unique_ptr<Command>> undo_;
  std::vector<std::unique_ptr<Command>> redo_;
};

}  // namespace bank
