#include <gtest/gtest.h>

#include "bank/Account.hpp"
#include "bank/Errors.hpp"

using namespace bank;


TEST(AccountState, NewAccountIsActive) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  EXPECT_EQ(acc.stateName(), "active");
  EXPECT_EQ(&acc.state(), &ActiveState::instance());
}

TEST(AccountState, FrozenAccountAcceptsDepositsButNoWithdrawals) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.deposit(euros(100));
  acc.freeze();
  EXPECT_EQ(acc.stateName(), "frozen");
  acc.deposit(euros(5));
  EXPECT_EQ(acc.balance(), euros(105));
  EXPECT_THROW(acc.withdraw(euros(1)), InvalidOperation);
  EXPECT_EQ(acc.balance(), euros(105));
}

TEST(AccountState, UnfreezeRestoresWithdrawals) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.deposit(euros(100));
  acc.freeze();
  acc.unfreeze();
  EXPECT_EQ(acc.stateName(), "active");
  acc.withdraw(euros(40));
  EXPECT_EQ(acc.balance(), euros(60));
}

TEST(AccountState, InvalidTransitionsAreRejected) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  EXPECT_THROW(acc.unfreeze(), InvalidOperation);  // not frozen
  acc.freeze();
  EXPECT_THROW(acc.freeze(), InvalidOperation);  // already frozen
  EXPECT_EQ(acc.stateName(), "frozen");
}

TEST(AccountState, CloseRequiresZeroBalance) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.deposit(euros(10));
  EXPECT_THROW(acc.close(), InvalidOperation);
  EXPECT_EQ(acc.stateName(), "active");
  acc.withdraw(euros(10));
  acc.close();
  EXPECT_EQ(acc.stateName(), "closed");
}

TEST(AccountState, FrozenAccountCanBeClosedWhenEmpty) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.freeze();
  acc.close();
  EXPECT_EQ(acc.stateName(), "closed");
}

TEST(AccountState, ClosedAccountRejectsEverything) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.close();
  EXPECT_THROW(acc.deposit(euros(1)), InvalidOperation);
  EXPECT_THROW(acc.withdraw(euros(1)), InvalidOperation);
  EXPECT_THROW(acc.freeze(), InvalidOperation);
  EXPECT_THROW(acc.unfreeze(), InvalidOperation);
  EXPECT_THROW(acc.close(), InvalidOperation);
}

TEST(AccountState, ErrorMessageNamesAccountAndState) {
  CheckingAccount acc("ACC-7", "Alice", euros(0));
  acc.freeze();
  try {
    acc.withdraw(euros(1));
    FAIL() << "expected InvalidOperation";
  } catch (const InvalidOperation& e) {
    EXPECT_STREQ(e.what(), "Cannot withdraw: account ACC-7 is frozen");
  }
}

TEST(AccountState, AmountIsValidatedBeforeTheState) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  acc.close();
  EXPECT_THROW(acc.deposit(euros(-1)), InvalidAmount);
}
