#include "bank/AccountState.hpp"

#include "bank/Account.hpp"
#include "bank/Errors.hpp"

namespace bank {

// --- base class: reject everything -----------------------------------------
void AccountState::deposit(Account& account, Money /*amount*/) const { reject(account, "deposit"); }
void AccountState::withdraw(Account& account, Money /*amount*/) const {
  reject(account, "withdraw");
}
void AccountState::freeze(Account& account) const { reject(account, "freeze"); }
void AccountState::unfreeze(Account& account) const { reject(account, "unfreeze"); }
void AccountState::close(Account& account) const { reject(account, "close"); }

void AccountState::credit(Account& account, Money amount) { account.balance_ += amount; }
void AccountState::debit(Account& account, Money amount) { account.balance_ -= amount; }
void AccountState::changeState(Account& account, const AccountState& next) {
  account.state_ = &next;
}

void AccountState::reject(const Account& account, const std::string& operation) const {
  throw InvalidOperation("Cannot " + operation + ": account " + account.id() + " is " + name());
}

namespace {
void requireZeroBalance(const Account& account) {
  if (account.balance() != Money()) {
    throw InvalidOperation("Cannot close account " + account.id() + ": balance is " +
                           account.balance().toString() + ", it must be 0.00");
  }
}
}  // namespace

// --- Active -----------------------------------------------------------------
const ActiveState& ActiveState::instance() {
  static const ActiveState state;
  return state;
}

void ActiveState::deposit(Account& account, Money amount) const { credit(account, amount); }

void ActiveState::withdraw(Account& account, Money amount) const {
  if (amount > account.availableFunds()) {
    throw InsufficientFunds("Cannot withdraw " + amount.toString() + " from " + account.id() +
                            ": only " + account.availableFunds().toString() + " available");
  }
  debit(account, amount);
}

void ActiveState::freeze(Account& account) const { changeState(account, FrozenState::instance()); }

void ActiveState::close(Account& account) const {
  requireZeroBalance(account);
  changeState(account, ClosedState::instance());
}

// --- Frozen -----------------------------------------------------------------
const FrozenState& FrozenState::instance() {
  static const FrozenState state;
  return state;
}

void FrozenState::deposit(Account& account, Money amount) const { credit(account, amount); }

void FrozenState::unfreeze(Account& account) const {
  changeState(account, ActiveState::instance());
}

void FrozenState::close(Account& account) const {
  requireZeroBalance(account);
  changeState(account, ClosedState::instance());
}

// --- Closed -----------------------------------------------------------------
const ClosedState& ClosedState::instance() {
  static const ClosedState state;
  return state;
}

}  // namespace bank
