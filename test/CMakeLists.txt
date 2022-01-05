add_executable(
  test_lexer
  test_lexer.cc
)
target_link_libraries(
  test_lexer
  gtest_main
)

add_executable(
  test_parser
  test_parser.cc
)
target_link_libraries(
  test_parser
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_lexer)
gtest_discover_tests(test_parser)

