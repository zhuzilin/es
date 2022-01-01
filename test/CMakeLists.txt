add_executable(
  test_lexer
  test_lexer.cc
)
target_link_libraries(
  test_lexer
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_lexer)
