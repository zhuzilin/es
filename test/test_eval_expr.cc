#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/evaluator.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string_view string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestEvalExpr, Number) {
  {
    std::vector<std::pair<string, double>> sources = {
      {u"0", 0}, {u"101", 101}, {u"0.01", 0.01}, {u"12.05", 12.05},
      {u".8", 0.8} ,{u"0xAbC09", 0xAbC09},
    };

    Evaluator eval;
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      Number* num = eval.EvalNumber(ast);
      EXPECT_EQ(pair.second, num->data());
    }
  }
}

TEST(TestEvalExpr, String) {
  {
    vec_pair_string sources = {
      {u"''", u""}, {u"\"\"", u""}, {u"'abc'", u"abc"},
      {u"'abc\\ndef'", u"abc\ndef"}, {u"'\\u4F60\\u597D'", u"你好"},
      {u"'\x24'", u"$"}, {u"'abc\\\ndef'", u"abcdef"},
    };

    Evaluator eval;
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      String* str = eval.EvalString(ast);
      EXPECT_EQ(pair.second, str->data());
    }
  }
}
