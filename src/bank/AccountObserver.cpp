#include "bank/AccountObserver.hpp"

#include <iomanip>

namespace bank {

std::string toString(EventType type) {
  switch (type) {
    case EventType::Deposit: return "deposit";
    case EventType::Withdrawal: return "withdrawal";
    case EventType::StateChanged: return "state change";
  }
  return "unknown";
}

void TransactionLogger::onEvent(const AccountEvent& e) {
  out_ << "[" << e.accountId << "] " << toString(e.type);
  if (e.type == EventType::StateChanged) {
    out_ << ' ' << e.detail;
  } else {
    out_ << ' ' << e.amount.toString() << " -> balance " << e.balanceAfter.toString();
  }
  out_ << '\n';
}

void LowBalanceAlert::onEvent(const AccountEvent& e) {
  if (e.type == EventType::Withdrawal && e.balanceAfter < threshold_) {
    ++alerts_;
    out_ << "ALERT [" << e.accountId << "] balance " << e.balanceAfter.toString()
         << " is below " << threshold_.toString() << '\n';
  }
}

void StatementRecorder::printStatement(std::ostream& out, const std::string& accountId) const {
  out << "Statement of " << accountId << '\n'
      << std::left << std::setw(14) << "event" << std::right << std::setw(12) << "amount"
      << std::setw(12) << "balance" << '\n';
  for (const auto& e : events_) {
    if (e.accountId != accountId) {
      continue;
    }
    const std::string amount = e.type == EventType::Withdrawal ? "-" + e.amount.toString()
                               : e.type == EventType::Deposit  ? "+" + e.amount.toString()
                                                               : "";
    out << std::left << std::setw(14) << toString(e.type) << std::right << std::setw(12)
        << amount << std::setw(12) << e.balanceAfter.toString();
    if (!e.detail.empty()) {
      out << "  (" << e.detail << ')';
    }
    out << '\n';
  }
}

}  // namespace bank
