# Core library with all the domain logic, used by the demo app and the tests.
add_library(bank_core STATIC
  bank/Account.cpp
  bank/AccountFactory.cpp
  bank/AccountState.cpp
  bank/Money.cpp
  bank/Version.cpp
)
target_include_directories(bank_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(bank_core PUBLIC cxx_std_17)
bank_set_warnings(bank_core)
bank_enable_coverage(bank_core)
