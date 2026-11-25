/// \file main.cpp
/// \brief bank_demo: a scripted walk-through of all design patterns.

#include <iostream>
#include <memory>

#include "bank/Bank.hpp"
#include "bank/Errors.hpp"
#include "bank/Version.hpp"

using namespace bank;

namespace {

void heading(const std::string& title) { std::cout << "\n=== " << title << " ===\n"; }

void printBalances(const Bank& bank) {
  for (const auto& id : bank.accountIds()) {
    const auto& a = bank.account(id);
    std::cout << "  " << id << "  " << a.owner() << " (" << toString(a.type()) << ", "
              << a.stateName() << "): " << a.balance().toString() << '\n';
  }
}

}  // namespace

int main() {
  std::cout << "BankSystem " << version() << " demo\n";

  Bank bank;  // accounts are created by the AccountFactory inside

  // Observer: three independent reactions to account events
  auto logger = std::make_shared<TransactionLogger>(std::cout);
  auto alert = std::make_shared<LowBalanceAlert>(euros(100), std::cout);
  auto statements = std::make_shared<StatementRecorder>();
  bank.addObserver(logger);
  bank.addObserver(alert);
  bank.addObserver(statements);

  heading("Factory: open accounts");
  const auto alice = bank.openAccount(AccountType::Checking, "Alice").id();
  const auto bob = bank.openAccount(AccountType::Savings, "Bob").id();
  printBalances(bank);

  heading("Command: transactions (observers log them)");
  bank.deposit(alice, euros(1000));
  bank.transfer(alice, bob, euros(250));
  bank.withdraw(alice, euros(900));  // uses the checking overdraft -> alert
  printBalances(bank);

  heading("Command: undo / redo");
  std::cout << "Undo list: ";
  for (const auto& d : bank.history().undoList()) {
    std::cout << "[" << d << "] ";
  }
  std::cout << '\n';
  bank.undo();  // the 900.00 withdrawal is reversed
  printBalances(bank);
  bank.redo();
  bank.undo();
  const Money interest = bank.payInterest(bob);
  std::cout << "Interest paid to Bob: " << interest.toString() << '\n';

  heading("State: freeze, try to withdraw, unfreeze");
  bank.freeze(alice);
  try {
    bank.withdraw(alice, euros(10));
  } catch (const InvalidOperation& e) {
    std::cout << "Rejected: " << e.what() << '\n';
  }
  bank.deposit(alice, euros(5));  // deposits still arrive on a frozen account
  bank.unfreeze(alice);

  heading("Errors are reported, not ignored");
  try {
    bank.withdraw(bob, euros(10000));
  } catch (const InsufficientFunds& e) {
    std::cout << "Rejected: " << e.what() << '\n';
  }
  try {
    bank.close(bob);
  } catch (const InvalidOperation& e) {
    std::cout << "Rejected: " << e.what() << '\n';
  }

  heading("Final balances");
  printBalances(bank);
  std::cout << "Total: " << bank.totalBalance().toString() << '\n';
  std::cout << "Low balance alerts: " << alert->alertCount() << '\n';

  heading("Observer: statement of " + alice);
  statements->printStatement(std::cout, alice);
  return 0;
}
