# Test coverage with gcov/lcov (GCC or Clang):
#   cmake -S . -B build-cov -DBANK_ENABLE_COVERAGE=ON
#   cmake --build build-cov
#   cmake --build build-cov --target coverage    -> build-cov/coverage/index.html

function(bank_enable_coverage target)
  if(BANK_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PUBLIC --coverage -O0 -g)
    target_link_options(${target} PUBLIC --coverage)
  endif()
endfunction()

if(BANK_ENABLE_COVERAGE)
  find_program(LCOV_EXE lcov)
  find_program(GENHTML_EXE genhtml)
  if(LCOV_EXE AND GENHTML_EXE)
    set(info ${PROJECT_BINARY_DIR}/coverage.info)
    add_custom_target(coverage
      COMMAND ${LCOV_EXE} --zerocounters --directory .
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      COMMAND ${LCOV_EXE} --capture --directory . --output-file ${info}
              --ignore-errors mismatch,inconsistent
      COMMAND ${LCOV_EXE} --extract ${info} "${PROJECT_SOURCE_DIR}/src/*"
              --output-file ${info} --ignore-errors unused
      COMMAND ${GENHTML_EXE} ${info} --output-directory coverage
      COMMAND ${LCOV_EXE} --summary ${info}
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Running the tests and generating the coverage report"
      VERBATIM)
  else()
    message(WARNING "BANK_ENABLE_COVERAGE=ON but lcov/genhtml were not found")
  endif()
endif()
