#include <gtest/gtest.h>

#include <sstream>

#include "bank/Bank.hpp"
#include "bank/Errors.hpp"

using namespace bank;

TEST(Bank, OpensAccountsWithSequentialIds) {
  Bank bank;
  auto& a = bank.openAccount(AccountType::Checking, "Alice");
  auto& b = bank.openAccount(AccountType::Savings, "Bob");
  EXPECT_EQ(a.id(), "ACC-0001");
  EXPECT_EQ(b.id(), "ACC-0002");
  EXPECT_EQ(&bank.account("ACC-0002"), &b);
  EXPECT_EQ(bank.accountIds(), (std::vector<std::string>{"ACC-0001", "ACC-0002"}));
  EXPECT_THROW((void)bank.account("ACC-9999"), AccountNotFound);
}

TEST(Bank, UsesTheFactorySettings) {
  Bank bank(AccountFactory(AccountDefaults{euros(50), 12.0}));
  auto& a = bank.openAccount(AccountType::Checking, "Alice");
  EXPECT_EQ(a.availableFunds(), euros(50));
}

TEST(Bank, TransactionsAndUndoAcrossAccounts) {
  Bank bank;
  const auto alice = bank.openAccount(AccountType::Checking, "Alice").id();
  const auto bob = bank.openAccount(AccountType::Savings, "Bob").id();
  bank.deposit(alice, euros(500));
  bank.transfer(alice, bob, euros(200));
  bank.withdraw(alice, euros(100));
  EXPECT_EQ(bank.account(alice).balance(), euros(200));
  EXPECT_EQ(bank.account(bob).balance(), euros(200));
  EXPECT_EQ(bank.totalBalance(), euros(400));

  EXPECT_TRUE(bank.undo());  // withdrawal
  EXPECT_TRUE(bank.undo());  // transfer
  EXPECT_EQ(bank.account(alice).balance(), euros(500));
  EXPECT_EQ(bank.account(bob).balance(), Money());
  EXPECT_TRUE(bank.redo());  // transfer again
  EXPECT_EQ(bank.account(bob).balance(), euros(200));
  EXPECT_EQ(bank.history().undoList().front(), "transfer 200.00 from ACC-0001 to ACC-0002");
}

TEST(Bank, InterestOnlyForSavings) {
  Bank bank;
  const auto checking = bank.openAccount(AccountType::Checking, "Alice").id();
  const auto savings = bank.openAccount(AccountType::Savings, "Bob").id();
  bank.deposit(savings, euros(1200));
  EXPECT_EQ(bank.payInterest(savings), euros(3));  // 1200 * 3% / 12
  EXPECT_THROW(bank.payInterest(checking), InvalidOperation);
}

TEST(Bank, ObserversSeeExistingAndFutureAccounts) {
  Bank bank;
  const auto first = bank.openAccount(AccountType::Checking, "Alice").id();
  auto recorder = std::make_shared<StatementRecorder>();
  bank.addObserver(recorder);
  bank.addObserver(nullptr);  // ignored
  const auto second = bank.openAccount(AccountType::Checking, "Bob").id();
  bank.deposit(first, euros(1));
  bank.deposit(second, euros(2));
  ASSERT_EQ(recorder->events().size(), 2U);
  EXPECT_EQ(recorder->events()[1].accountId, second);
}

TEST(Bank, AdministrativeActions) {
  Bank bank;
  const auto id = bank.openAccount(AccountType::Checking, "Alice").id();
  bank.freeze(id);
  EXPECT_THROW(bank.withdraw(id, euros(1)), InvalidOperation);
  EXPECT_FALSE(bank.history().canUndo());
  bank.unfreeze(id);
  bank.close(id);
  EXPECT_EQ(bank.account(id).stateName(), "closed");
}
