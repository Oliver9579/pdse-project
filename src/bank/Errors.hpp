/// \file Errors.hpp
/// \brief Exception types of the bank domain.
#pragma once

#include <stdexcept>
#include <string>

namespace bank {

/// Base class of all bank errors, so callers can catch them in one place.
class BankError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

/// An amount is not valid for the operation (e.g. zero or negative deposit).
class InvalidAmount : public BankError {
 public:
  using BankError::BankError;
};

/// A withdrawal would exceed the available funds.
class InsufficientFunds : public BankError {
 public:
  using BankError::BankError;
};

/// The operation is not allowed in the current account state (e.g. frozen).
class InvalidOperation : public BankError {
 public:
  using BankError::BankError;
};

/// No account with the given id exists.
class AccountNotFound : public BankError {
 public:
  using BankError::BankError;
};

}  // namespace bank
