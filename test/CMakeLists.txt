cmake_minimum_required(VERSION 3.2)

add_executable(main_test test.cpp)

target_link_libraries( main_test
  gtest
  gtest_main
  pthread
)

add_test(
  NAME main_test
  COMMAND $<TARGET_FILE:main_test>
)

set_property(
    TEST main_test
    PROPERTY LABELS main_test main_test
)
