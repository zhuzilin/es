#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestProgram, SimpleAssign0) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"a = 1;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Number* num = static_cast<Number*>(GetValue(e, static_cast<Reference*>(res.value)));
    EXPECT_EQ(1, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, SimpleAssign1) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"a = 1;a=2;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, static_cast<Reference*>(res.value)));
    EXPECT_EQ(2, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, Call0) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"a = function(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(3, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, Call1) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"function a(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(3, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, Call2) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"a = 1; function b(){return a;}; b()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(1, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, Call3) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"function c(){return function() { return 10};}; c()()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(10, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, CallFunctionContructor) {
  Init();
  {
    Error* e = nullptr;
    Parser parser(u"a = Function('return 5'); a()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(nullptr, e);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(5, num->data());
  }
}
