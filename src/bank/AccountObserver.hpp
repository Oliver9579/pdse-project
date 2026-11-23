/// \file AccountObserver.hpp
/// \brief Notifications about account activity (Observer pattern).
#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "bank/Money.hpp"

namespace bank {

enum class EventType { Deposit, Withdrawal, StateChanged };

/// "deposit", "withdrawal" or "state change".
std::string toString(EventType type);

/// \brief Something that happened to an account.
struct AccountEvent {
  EventType type;
  std::string accountId;
  Money amount;          ///< Zero for state changes
  Money balanceAfter;    ///< Balance after the event
  std::string detail;    ///< e.g. "active -> frozen"
};

/// \brief Design pattern: **Observer**.
///
/// An Account (the *subject*) notifies all registered observers after every
/// successful deposit, withdrawal or state change. The account does not know
/// what the observers do - logging, alerting, keeping a statement - so new
/// reactions can be added without touching Account (loose coupling,
/// Open-Closed Principle).
class AccountObserver {
 public:
  AccountObserver() = default;
  virtual ~AccountObserver() = default;
  AccountObserver(const AccountObserver&) = delete;
  AccountObserver& operator=(const AccountObserver&) = delete;
  AccountObserver(AccountObserver&&) = delete;
  AccountObserver& operator=(AccountObserver&&) = delete;

  virtual void onEvent(const AccountEvent& event) = 0;
};

/// \brief Writes one line per event, e.g. "[ACC-0001] deposit 100.00 -> balance 100.00".
class TransactionLogger final : public AccountObserver {
 public:
  explicit TransactionLogger(std::ostream& out) : out_(out) {}
  void onEvent(const AccountEvent& event) override;

 private:
  std::ostream& out_;
};

/// \brief Warns when a withdrawal leaves the balance below a threshold.
class LowBalanceAlert final : public AccountObserver {
 public:
  LowBalanceAlert(Money threshold, std::ostream& out) : threshold_(threshold), out_(out) {}
  void onEvent(const AccountEvent& event) override;
  std::size_t alertCount() const { return alerts_; }

 private:
  Money threshold_;
  std::ostream& out_;
  std::size_t alerts_ = 0;
};

/// \brief Keeps every event in memory and prints a bank statement.
class StatementRecorder final : public AccountObserver {
 public:
  void onEvent(const AccountEvent& event) override { events_.push_back(event); }
  const std::vector<AccountEvent>& events() const { return events_; }
  /// Prints the events of one account as a table.
  void printStatement(std::ostream& out, const std::string& accountId) const;

 private:
  std::vector<AccountEvent> events_;
};

}  // namespace bank
