#include <gtest/gtest.h>

#include "bank/Account.hpp"
#include "bank/AccountFactory.hpp"
#include "bank/Errors.hpp"

using namespace bank;

TEST(Account, DepositAndWithdraw) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.deposit(euros(100));
  acc.withdraw(euros(30, 50));
  EXPECT_EQ(acc.balance(), euros(69, 50));
}

TEST(Account, RejectsNonPositiveAmounts) {
  CheckingAccount acc("ACC-1", "Alice", euros(100));
  EXPECT_THROW(acc.deposit(Money()), InvalidAmount);
  EXPECT_THROW(acc.deposit(euros(-5)), InvalidAmount);
  EXPECT_THROW(acc.withdraw(Money()), InvalidAmount);
  EXPECT_EQ(acc.balance(), Money());
}

TEST(Account, RequiresIdAndOwner) {
  EXPECT_THROW(CheckingAccount("", "Alice", euros(0)), BankError);
  EXPECT_THROW(SavingsAccount("ACC-1", "", 1.0), BankError);
}

TEST(CheckingAccount, CanUseOverdraftButNotMore) {
  CheckingAccount acc("ACC-1", "Alice", euros(200));
  acc.deposit(euros(50));
  EXPECT_EQ(acc.availableFunds(), euros(250));
  acc.withdraw(euros(250));
  EXPECT_EQ(acc.balance(), euros(-200));
  EXPECT_THROW(acc.withdraw(euros(0, 1)), InsufficientFunds);
  EXPECT_EQ(acc.balance(), euros(-200));  // unchanged after a failed withdrawal
  EXPECT_THROW(CheckingAccount("ACC-2", "Bob", euros(-1)), InvalidAmount);
}

TEST(SavingsAccount, CannotGoBelowZero) {
  SavingsAccount acc("ACC-1", "Alice", 2.0);
  acc.deposit(euros(100));
  EXPECT_THROW(acc.withdraw(euros(100, 1)), InsufficientFunds);
  acc.withdraw(euros(100));
  EXPECT_EQ(acc.balance(), Money());
}

TEST(SavingsAccount, MonthlyInterestIsRoundedToTheCent) {
  SavingsAccount acc("ACC-1", "Alice", 3.0);
  EXPECT_EQ(acc.monthlyInterest(), Money());  // nothing on an empty account
  acc.deposit(euros(1000));
  EXPECT_EQ(acc.monthlyInterest(), euros(2, 50));  // 1000 * 3% / 12
  acc.withdraw(euros(999, 99));
  EXPECT_EQ(acc.monthlyInterest(), Money());  // 0.01 * 0.25% rounds to 0
  EXPECT_THROW(SavingsAccount("ACC-2", "Bob", -1.0), BankError);
  EXPECT_THROW(SavingsAccount("ACC-2", "Bob", 150.0), BankError);
}

TEST(AccountFactory, CreatesConfiguredAccounts) {
  const AccountFactory factory(AccountDefaults{euros(300), 4.5});
  const auto checking = factory.create(AccountType::Checking, "ACC-1", "Alice");
  const auto savings = factory.create("savings", "ACC-2", "Bob");
  EXPECT_EQ(checking->type(), AccountType::Checking);
  EXPECT_EQ(checking->availableFunds(), euros(300));
  EXPECT_EQ(savings->type(), AccountType::Savings);
  EXPECT_EQ(savings->owner(), "Bob");
  const auto* s = dynamic_cast<const SavingsAccount*>(savings.get());
  ASSERT_NE(s, nullptr);
  EXPECT_DOUBLE_EQ(s->annualInterestPercent(), 4.5);
}

TEST(AccountFactory, ParsesTypeNames) {
  EXPECT_EQ(AccountFactory::parseType("checking"), AccountType::Checking);
  EXPECT_EQ(toString(AccountType::Savings), "savings");
  EXPECT_THROW(AccountFactory::parseType("crypto"), BankError);
  EXPECT_THROW(AccountFactory().create("crypto", "ACC-1", "Alice"), BankError);
}
