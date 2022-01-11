#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/eval.h>
#include <es/enter_code.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string_view string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestEvalExpr, Number) {
  Error* e = nullptr;
  {
    std::vector<std::pair<string, double>> sources = {
      {u"0", 0}, {u"101", 101}, {u"0.01", 0.01}, {u"12.05", 12.05},
      {u".8", 0.8} ,{u"0xAbC09", 0xAbC09},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EnterGlobalCode(e, ast);
      Number* num = EvalNumber(ast);
      EXPECT_EQ(pair.second, num->data());
    }
  }
}

TEST(TestEvalExpr, String) {
  Error* e = nullptr;
  {
    vec_pair_string sources = {
      {u"''", u""}, {u"\"\"", u""}, {u"'abc'", u"abc"},
      {u"'abc\\ndef'", u"abc\ndef"}, {u"'\\u4F60\\u597D'", u"你好"},
      {u"'\x24'", u"$"}, {u"'abc\\\ndef'", u"abcdef"},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EnterGlobalCode(e, ast);
      String* str = EvalString(ast);
      EXPECT_EQ(pair.second, str->data());
    }
  }
}

TEST(TestEvalExpr, Identifier) {
  Error* e = nullptr;
  {
    Parser parser(u"a");
    AST* ast = parser.ParsePrimaryExpression();
    EnterGlobalCode(e, ast);
    Reference* ref = EvalIdentifier(ast);
    EXPECT_EQ(nullptr, GetValue(e, ref));
  }
}

TEST(TestEvalExpr, SimpleAssign) {
  Error* e = nullptr;
  {
    Parser parser(u"a = 1");
    AST* ast = parser.ParseAssignmentExpression(false);
    EnterGlobalCode(e, ast);
    JSValue* val = EvalBinaryExpression(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, val->type());
    Number* num = static_cast<Number*>(val);
    EXPECT_EQ(1, num->data());
  }
}

TEST(TestEvalExpr, Function) {
  Error* e = nullptr;
  {
    Parser parser(u"function (b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    EnterGlobalCode(e, ast);
    JSValue* val = EvalFunction(ast);
    EXPECT_EQ(JSValue::JS_OBJECT, val->type());
    FunctionObject* func = static_cast<FunctionObject*>(val);
    EXPECT_EQ(1, func->FormalParameters().size());
  }

  {
    Parser parser(u"function a(b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    JSValue* val = EvalFunction(ast);
    EXPECT_EQ(JSValue::JS_OBJECT, val->type());
    FunctionObject* func = static_cast<FunctionObject*>(val);
    EXPECT_EQ(1, func->FormalParameters().size());
  }
}
