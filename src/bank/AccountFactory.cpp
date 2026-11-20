#include "bank/AccountFactory.hpp"

#include "bank/Errors.hpp"

namespace bank {

std::unique_ptr<Account> AccountFactory::create(AccountType type, std::string id,
                                                std::string owner) const {
  switch (type) {
    case AccountType::Checking:
      return std::make_unique<CheckingAccount>(std::move(id), std::move(owner),
                                               defaults_.checkingOverdraft);
    case AccountType::Savings:
      return std::make_unique<SavingsAccount>(std::move(id), std::move(owner),
                                              defaults_.savingsInterestPercent);
  }
  throw BankError("Unknown account type");
}

std::unique_ptr<Account> AccountFactory::create(const std::string& type, std::string id,
                                                std::string owner) const {
  return create(parseType(type), std::move(id), std::move(owner));
}

AccountType AccountFactory::parseType(const std::string& type) {
  if (type == "checking") {
    return AccountType::Checking;
  }
  if (type == "savings") {
    return AccountType::Savings;
  }
  throw BankError("Unknown account type '" + type + "' (use 'checking' or 'savings')");
}

}  // namespace bank
