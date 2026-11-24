#include "bank/Command.hpp"

#include "bank/Errors.hpp"

namespace bank {

std::string DepositCommand::describe() const {
  return "deposit " + amount_.toString() + " to " + account_.id();
}

std::string WithdrawCommand::describe() const {
  return "withdraw " + amount_.toString() + " from " + account_.id();
}

TransferCommand::TransferCommand(Account& from, Account& to, Money amount)
    : from_(from), to_(to), amount_(amount) {
  if (&from_ == &to_) {
    throw InvalidOperation("Cannot transfer from account " + from_.id() + " to itself");
  }
}

void TransferCommand::execute() {
  from_.withdraw(amount_);
  try {
    to_.deposit(amount_);
  } catch (...) {
    from_.deposit(amount_);  // roll back: all or nothing
    throw;
  }
}

void TransferCommand::undo() {
  to_.withdraw(amount_);
  try {
    from_.deposit(amount_);
  } catch (...) {
    to_.deposit(amount_);
    throw;
  }
}

std::string TransferCommand::describe() const {
  return "transfer " + amount_.toString() + " from " + from_.id() + " to " + to_.id();
}

void InterestCommand::execute() {
  if (!computed_) {
    amount_ = account_.monthlyInterest();
    computed_ = true;
  }
  if (!amount_.isPositive()) {
    throw InvalidAmount("No interest to pay on account " + account_.id());
  }
  account_.deposit(amount_);
}

void InterestCommand::undo() { account_.withdraw(amount_); }

std::string InterestCommand::describe() const {
  return "interest " + amount_.toString() + " to " + account_.id();
}

void CommandHistory::execute(std::unique_ptr<Command> command) {
  if (!command) {
    throw BankError("CommandHistory::execute: null command");
  }
  command->execute();  // throws -> nothing recorded
  undo_.push_back(std::move(command));
  redo_.clear();
}

bool CommandHistory::undo() {
  if (undo_.empty()) {
    return false;
  }
  undo_.back()->undo();  // throws -> history unchanged
  redo_.push_back(std::move(undo_.back()));
  undo_.pop_back();
  return true;
}

bool CommandHistory::redo() {
  if (redo_.empty()) {
    return false;
  }
  redo_.back()->execute();
  undo_.push_back(std::move(redo_.back()));
  redo_.pop_back();
  return true;
}

std::vector<std::string> CommandHistory::undoList() const {
  std::vector<std::string> list;
  for (auto it = undo_.rbegin(); it != undo_.rend(); ++it) {
    list.push_back((*it)->describe());
  }
  return list;
}

}  // namespace bank
