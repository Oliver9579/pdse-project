add_executable(bank_demo main.cpp)
target_link_libraries(bank_demo PRIVATE bank_core)
bank_set_warnings(bank_demo)
