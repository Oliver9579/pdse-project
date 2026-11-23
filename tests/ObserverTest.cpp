#include <gtest/gtest.h>

#include <sstream>

#include "bank/Account.hpp"
#include "bank/AccountObserver.hpp"
#include "bank/Errors.hpp"

using namespace bank;

namespace {
/// Test double: remembers the events it received.
class SpyObserver : public AccountObserver {
 public:
  void onEvent(const AccountEvent& e) override { events.push_back(e); }
  std::vector<AccountEvent> events;
};
}  // namespace

TEST(Observer, NotifiedAfterDepositWithdrawalAndStateChange) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  auto spy = std::make_shared<SpyObserver>();
  acc.addObserver(spy);
  acc.deposit(euros(100));
  acc.withdraw(euros(30));
  acc.freeze();
  ASSERT_EQ(spy->events.size(), 3U);
  EXPECT_EQ(spy->events[0].type, EventType::Deposit);
  EXPECT_EQ(spy->events[0].amount, euros(100));
  EXPECT_EQ(spy->events[1].type, EventType::Withdrawal);
  EXPECT_EQ(spy->events[1].balanceAfter, euros(70));
  EXPECT_EQ(spy->events[2].type, EventType::StateChanged);
  EXPECT_EQ(spy->events[2].detail, "active -> frozen");
  EXPECT_EQ(spy->events[2].accountId, "ACC-1");
}

TEST(Observer, NotNotifiedWhenOperationFails) {
  SavingsAccount acc("ACC-1", "Alice", 1.0);
  auto spy = std::make_shared<SpyObserver>();
  acc.addObserver(spy);
  EXPECT_THROW(acc.withdraw(euros(1)), InsufficientFunds);
  EXPECT_THROW(acc.unfreeze(), InvalidOperation);
  EXPECT_TRUE(spy->events.empty());
}

TEST(Observer, AllObserversAreNotifiedAndCanBeRemoved) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  auto a = std::make_shared<SpyObserver>();
  auto b = std::make_shared<SpyObserver>();
  acc.addObserver(a);
  acc.addObserver(b);
  acc.addObserver(nullptr);  // ignored
  EXPECT_EQ(acc.observerCount(), 2U);
  acc.deposit(euros(1));
  acc.removeObserver(a);
  acc.deposit(euros(1));
  EXPECT_EQ(a->events.size(), 1U);
  EXPECT_EQ(b->events.size(), 2U);
  EXPECT_EQ(acc.observerCount(), 1U);
}

TEST(Observer, AccountDoesNotKeepObserversAlive) {
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  {
    auto temporary = std::make_shared<SpyObserver>();
    acc.addObserver(temporary);
    EXPECT_EQ(acc.observerCount(), 1U);
  }  // observer destroyed here
  EXPECT_EQ(acc.observerCount(), 0U);
  EXPECT_NO_THROW(acc.deposit(euros(1)));  // no dangling pointer
}

TEST(TransactionLogger, WritesOneLinePerEvent) {
  std::ostringstream out;
  CheckingAccount acc("ACC-1", "Alice", euros(0));
  auto logger = std::make_shared<TransactionLogger>(out);
  acc.addObserver(logger);
  acc.deposit(euros(100));
  acc.withdraw(euros(25, 50));
  acc.freeze();
  EXPECT_EQ(out.str(),
            "[ACC-1] deposit 100.00 -> balance 100.00\n"
            "[ACC-1] withdrawal 25.50 -> balance 74.50\n"
            "[ACC-1] state change active -> frozen\n");
}

TEST(LowBalanceAlert, FiresOnlyForWithdrawalsBelowThreshold) {
  std::ostringstream out;
  CheckingAccount acc("ACC-1", "Alice", euros(100));
  auto alert = std::make_shared<LowBalanceAlert>(euros(50), out);
  acc.addObserver(alert);
  acc.deposit(euros(60));
  acc.withdraw(euros(5));   // 55: fine
  acc.withdraw(euros(10));  // 45: alert
  acc.deposit(euros(1));    // deposits never alert
  acc.withdraw(euros(80));  // -34: alert
  EXPECT_EQ(alert->alertCount(), 2U);
  EXPECT_NE(out.str().find("ALERT [ACC-1] balance 45.00 is below 50.00"), std::string::npos);
}

TEST(StatementRecorder, PrintsOnlyTheRequestedAccount) {
  CheckingAccount a("ACC-1", "Alice", euros(0));
  CheckingAccount b("ACC-2", "Bob", euros(0));
  auto recorder = std::make_shared<StatementRecorder>();
  a.addObserver(recorder);
  b.addObserver(recorder);
  a.deposit(euros(10));
  b.deposit(euros(20));
  a.withdraw(euros(3));
  EXPECT_EQ(recorder->events().size(), 3U);
  std::ostringstream out;
  recorder->printStatement(out, "ACC-1");
  EXPECT_NE(out.str().find("+10.00"), std::string::npos);
  EXPECT_NE(out.str().find("-3.00"), std::string::npos);
  EXPECT_EQ(out.str().find("20.00"), std::string::npos);
  EXPECT_EQ(toString(EventType::StateChanged), "state change");
}
