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

add_executable(
  test_eval_expr
  test_eval_expr.cc
)
target_link_libraries(
  test_eval_expr
  gtest_main
)

add_executable(
  test_primitive_conversion
  test_primitive_conversion.cc
)
target_link_libraries(
  test_primitive_conversion
  gtest_main
)

add_executable(
  test_same_value
  test_same_value.cc
)
target_link_libraries(
  test_same_value
  gtest_main
)

add_executable(
  test_program
  test_program.cc
)
target_link_libraries(
  test_program
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_lexer)
gtest_discover_tests(test_parser)
gtest_discover_tests(test_eval_expr)
gtest_discover_tests(test_primitive_conversion)
gtest_discover_tests(test_same_value)
gtest_discover_tests(test_program)
