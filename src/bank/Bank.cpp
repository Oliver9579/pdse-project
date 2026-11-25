#include "bank/Bank.hpp"

#include <cstdio>

#include "bank/Errors.hpp"

namespace bank {

Bank::Bank(AccountFactory factory) : factory_(std::move(factory)) {}

Account& Bank::openAccount(AccountType type, const std::string& owner) {
  char id[16];
  std::snprintf(id, sizeof id, "ACC-%04d", nextNumber_++);
  auto created = factory_.create(type, id, owner);
  for (const auto& o : observers_) {
    created->addObserver(o);
  }
  Account& ref = *created;
  accounts_.emplace(ref.id(), std::move(created));
  return ref;
}

Account& Bank::account(const std::string& id) {
  const auto it = accounts_.find(id);
  if (it == accounts_.end()) {
    throw AccountNotFound("No account with id '" + id + "'");
  }
  return *it->second;
}

const Account& Bank::account(const std::string& id) const {
  return const_cast<Bank*>(this)->account(id);
}

std::vector<std::string> Bank::accountIds() const {
  std::vector<std::string> ids;
  for (const auto& entry : accounts_) {
    ids.push_back(entry.first);
  }
  return ids;
}

Money Bank::totalBalance() const {
  Money total;
  for (const auto& entry : accounts_) {
    total += entry.second->balance();
  }
  return total;
}

void Bank::deposit(const std::string& id, Money amount) {
  history_.execute(std::make_unique<DepositCommand>(account(id), amount));
}

void Bank::withdraw(const std::string& id, Money amount) {
  history_.execute(std::make_unique<WithdrawCommand>(account(id), amount));
}

void Bank::transfer(const std::string& from, const std::string& to, Money amount) {
  history_.execute(std::make_unique<TransferCommand>(account(from), account(to), amount));
}

Money Bank::payInterest(const std::string& id) {
  auto* savings = dynamic_cast<SavingsAccount*>(&account(id));
  if (savings == nullptr) {
    throw InvalidOperation("Account " + id + " is not a savings account");
  }
  auto command = std::make_unique<InterestCommand>(*savings);
  auto* interest = command.get();
  history_.execute(std::move(command));
  return interest->amount();
}

void Bank::addObserver(const std::shared_ptr<AccountObserver>& observer) {
  if (!observer) {
    return;
  }
  observers_.push_back(observer);  // the bank keeps its observers alive
  for (auto& entry : accounts_) {
    entry.second->addObserver(observer);
  }
}

}  // namespace bank
