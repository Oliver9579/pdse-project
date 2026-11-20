# GoogleTest: use the installed version if available, otherwise download it.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

add_executable(bank_tests
  AccountTest.cpp
  MoneyTest.cpp
  VersionTest.cpp
)
target_link_libraries(bank_tests PRIVATE bank_core GTest::gtest_main)
bank_enable_coverage(bank_tests)
gtest_discover_tests(bank_tests)
