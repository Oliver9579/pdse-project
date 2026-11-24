#include <gtest/gtest.h>

#include "bank/Command.hpp"
#include "bank/Errors.hpp"

using namespace bank;

class CommandTest : public ::testing::Test {
 protected:
  CheckingAccount alice{"ACC-1", "Alice", euros(0)};
  CheckingAccount bob{"ACC-2", "Bob", euros(0)};
  CommandHistory history;

  void SetUp() override { alice.deposit(euros(100)); }
};

TEST_F(CommandTest, DepositAndWithdrawCanBeUndone) {
  history.execute(std::make_unique<DepositCommand>(alice, euros(50)));
  history.execute(std::make_unique<WithdrawCommand>(alice, euros(20)));
  EXPECT_EQ(alice.balance(), euros(130));
  EXPECT_TRUE(history.undo());
  EXPECT_EQ(alice.balance(), euros(150));
  EXPECT_TRUE(history.undo());
  EXPECT_EQ(alice.balance(), euros(100));
  EXPECT_FALSE(history.undo());  // nothing left
}

TEST_F(CommandTest, RedoReappliesUndoneCommands) {
  history.execute(std::make_unique<DepositCommand>(alice, euros(50)));
  history.undo();
  EXPECT_TRUE(history.canRedo());
  EXPECT_TRUE(history.redo());
  EXPECT_EQ(alice.balance(), euros(150));
  EXPECT_FALSE(history.redo());
}

TEST_F(CommandTest, NewCommandClearsRedo) {
  history.execute(std::make_unique<DepositCommand>(alice, euros(50)));
  history.undo();
  history.execute(std::make_unique<DepositCommand>(alice, euros(1)));
  EXPECT_FALSE(history.canRedo());
}

TEST_F(CommandTest, FailedCommandIsNotRecorded) {
  EXPECT_THROW(history.execute(std::make_unique<WithdrawCommand>(alice, euros(1000))),
               InsufficientFunds);
  EXPECT_FALSE(history.canUndo());
  EXPECT_EQ(alice.balance(), euros(100));
  EXPECT_THROW(history.execute(nullptr), BankError);
}

TEST_F(CommandTest, TransferMovesMoneyAndUndoMovesItBack) {
  history.execute(std::make_unique<TransferCommand>(alice, bob, euros(40)));
  EXPECT_EQ(alice.balance(), euros(60));
  EXPECT_EQ(bob.balance(), euros(40));
  history.undo();
  EXPECT_EQ(alice.balance(), euros(100));
  EXPECT_EQ(bob.balance(), Money());
}

TEST_F(CommandTest, TransferIsAllOrNothing) {
  bob.close();  // bob cannot receive money any more
  EXPECT_THROW(history.execute(std::make_unique<TransferCommand>(alice, bob, euros(40))),
               InvalidOperation);
  EXPECT_EQ(alice.balance(), euros(100));  // withdrawal was rolled back
  EXPECT_FALSE(history.canUndo());
}

TEST_F(CommandTest, TransferToSameAccountIsRejected) {
  EXPECT_THROW(TransferCommand(alice, alice, euros(1)), InvalidOperation);
}

TEST_F(CommandTest, UndoRespectsAccountStateAndKeepsHistory) {
  history.execute(std::make_unique<DepositCommand>(alice, euros(50)));
  alice.freeze();
  // undoing the deposit means withdrawing, which a frozen account forbids
  EXPECT_THROW(history.undo(), InvalidOperation);
  EXPECT_TRUE(history.canUndo());  // still undoable later
  alice.unfreeze();
  EXPECT_TRUE(history.undo());
  EXPECT_EQ(alice.balance(), euros(100));
}

TEST_F(CommandTest, UndoListShowsMostRecentFirst) {
  history.execute(std::make_unique<DepositCommand>(alice, euros(5)));
  history.execute(std::make_unique<TransferCommand>(alice, bob, euros(2, 50)));
  EXPECT_EQ(history.undoList(),
            (std::vector<std::string>{"transfer 2.50 from ACC-1 to ACC-2", "deposit 5.00 to ACC-1"}));
  EXPECT_EQ(WithdrawCommand(bob, euros(1)).describe(), "withdraw 1.00 from ACC-2");
}

TEST(InterestCommand, PaysInterestOnceAndRemembersAmountForRedo) {
  SavingsAccount savings("ACC-3", "Carol", 6.0);
  savings.deposit(euros(1000));
  CommandHistory history;
  auto cmd = std::make_unique<InterestCommand>(savings);
  auto* interest = cmd.get();
  history.execute(std::move(cmd));
  EXPECT_EQ(interest->amount(), euros(5));  // 1000 * 6% / 12
  EXPECT_EQ(savings.balance(), euros(1005));
  EXPECT_EQ(interest->describe(), "interest 5.00 to ACC-3");
  history.undo();
  EXPECT_EQ(savings.balance(), euros(1000));
  history.redo();
  EXPECT_EQ(savings.balance(), euros(1005));  // same 5.00, not recomputed on 1005

  SavingsAccount empty("ACC-4", "Dan", 6.0);
  EXPECT_THROW(history.execute(std::make_unique<InterestCommand>(empty)), InvalidAmount);
}
