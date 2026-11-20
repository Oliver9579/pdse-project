/// \file AccountFactory.hpp
/// \brief Creates accounts without the caller knowing the concrete classes.
#pragma once

#include <memory>
#include <string>

#include "bank/Account.hpp"

namespace bank {

/// \brief Default product parameters used by the factory.
struct AccountDefaults {
  Money checkingOverdraft = euros(500);  ///< Overdraft limit of new checking accounts
  double savingsInterestPercent = 3.0;   ///< Annual interest of new savings accounts
};

/// \brief Design pattern: **Factory**.
///
/// The rest of the program asks for "a savings account" and gets back an
/// `Account` pointer; only this class knows the concrete subclasses and how
/// to configure them. Benefits:
///  * clients depend on the abstraction (Account), not on concrete classes
///    (Dependency Inversion Principle),
///  * object creation and its default parameters live in one place,
///  * adding a new account type touches the factory, not every caller.
class AccountFactory {
 public:
  explicit AccountFactory(AccountDefaults defaults = {}) : defaults_(defaults) {}

  std::unique_ptr<Account> create(AccountType type, std::string id, std::string owner) const;
  /// Same, with the type given as text ("checking" / "savings"), e.g. from user input.
  std::unique_ptr<Account> create(const std::string& type, std::string id, std::string owner) const;

  /// Parses "checking" / "savings"; throws BankError otherwise.
  static AccountType parseType(const std::string& type);

  const AccountDefaults& defaults() const { return defaults_; }

 private:
  AccountDefaults defaults_;
};

}  // namespace bank
