#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/eval.h>
#include <es/enter_code.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/utils/helper.h>
#include <es/gc/heap.h>
#include <es/impl.h>
#include <es/eval.h>
#include <es/enter_code.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestEvalExpr, Number) {
  JSValue e = error::Ok();
  {
    std::vector<std::pair<string, double>> sources = {
      {u"0", 0}, {u"101", 101}, {u"0.01", 0.01}, {u"12.05", 12.05},
      {u".8", 0.8} ,{u"0xAbC09", 0xAbC09},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      JSValue num = EvalNumber(ast);
      EXPECT_EQ(pair.second, number::data(num));
    }
  }
}

TEST(TestEvalExpr, Arithmetic) {
  Init();
  JSValue e = error::Ok();
  EnterGlobalCode(e, new ProgramOrFunctionBody(AST::AST_PROGRAM, false));
  {
    std::vector<std::pair<string, double>> sources = {
      {u"2 * 3", 6}, {u"-2 * 6", -12},
      {u"1 / 2", 0.5}, {u"-9 / 3", -3},
      {u"-4 % 3", -1}, {u"4 % 3", 1},
      {u"1 + 2", 3}, {u"0 + -5", -5},
      {u"1 - 2", -1}, {u"0 - -5", 5}
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      JSValue val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(Type::JS_NUMBER, val.type());
      EXPECT_EQ(pair.second, number::data(val));
    }
  }

  {
    vec_string sources = {
      u"NaN * 3", u"3 * NaN", u"NaN * NaN", u"Infinity * 0",
      u"NaN / 3", u"3 / NaN", u"NaN / NaN", u"Infinity / Infinity", u"0 / 0",
      u"NaN % 3", u"3 % NaN", u"NaN % NaN", u"Infinity % 3", u"1 % 0",
      u"NaN + 3", u"3 + NaN", u"NaN + NaN", u"Infinity + -Infinity",
      u"NaN - 3", u"3 - NaN", u"NaN - NaN", u"Infinity - Infinity",
    };

    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      JSValue val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(Type::JS_NUMBER, val.type());
      EXPECT_EQ(true, isnan(number::data(val)));
    }
  }

  {
    std::vector<std::pair<string, double>> sources = {
      {u"Infinity * Infinity", 0}, {u"Infinity * -Infinity", 1},
      {u"3 * Infinity", 0}, {u"-5 * Infinity", 1},
      {u"Infinity / 3", 0}, {u"-Infinity / 5", 1}, {u"1 / -0", 1},
      {u"Infinity + Infinity", 0},
      {u"Infinity - - Infinity", 0}
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      JSValue val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(Type::JS_NUMBER, val.type());
      EXPECT_EQ(true, isinf(number::data(val)));
      EXPECT_EQ(pair.second, signbit(number::data(val)));
    }
  }

  {
    std::vector<std::pair<string, double>> sources = {
      {u"-0 + -0", 1}, {u"-0 + 0", 0}, {u"0 + 0", 0}, {u"-1 + 1", 0},
      {u"-0 - 0", 1}, {u"-0 - -0", 0}, {u"0 - -0", 0}, {u"-1 - -1", 0},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      JSValue val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(Type::JS_NUMBER, val.type());
      EXPECT_EQ(0, number::data(val));
      EXPECT_EQ(pair.second, signbit(number::data(val)));
    }
  }
}

TEST(TestEvalExpr, String) {
  JSValue e = error::Ok();
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
      Handle<String> str = EvalString(e, ast);
      EXPECT_EQ(pair.second, str.val()->data());
    }
  }

  {
    vec_pair_string sources = {
      {u"'a' + 'b'", u"ab"},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      JSValue val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(Type::JS_STRING, val.type());
      Handle<String> str = static_cast<Handle<String>>(val);
      EXPECT_EQ(pair.second, str.val()->data());
    }
  }
}

TEST(TestEvalExpr, Identifier) {
  Init();
  JSValue e = error::Ok();
  {
    Parser parser(u"a");
    AST* ast = parser.ParsePrimaryExpression();
    EnterGlobalCode(e, ast);
    JSValue ref = EvalIdentifier(ast);
    EXPECT_EQ(true, GetValue(e, ref).IsNullptr());
  }
}

TEST(TestEvalExpr, SimpleAssign) {
  Init();
  JSValue e = error::Ok();
  {
    Parser parser(u"a = 1");
    AST* ast = parser.ParseAssignmentExpression(false);
    EnterGlobalCode(e, ast);
    JSValue val = EvalBinaryExpression(e, ast);
    EXPECT_EQ(Type::JS_NUMBER, val.type());
    EXPECT_EQ(1, number::data(val));
  }
}

TEST(TestEvalExpr, Function) {
  Init();
  JSValue e = error::Ok();
  {
    Parser parser(u"function (b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    EnterGlobalCode(e, ast);
    JSValue val = EvalFunction(e, ast);
    EXPECT_EQ(JSValue::OBJ_FUNC, val.type());
    EXPECT_EQ(1,  fixed_array::size(function_object::FormalParameters(val)));
  }

  {
    Parser parser(u"function a(b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    JSValue val = EvalFunction(e, ast);
    EXPECT_EQ(JSValue::OBJ_FUNC, val.type());
    EXPECT_EQ(1, fixed_array::size(function_object::FormalParameters(val)));
  }
}

TEST(TestEvalExpr, Object) {
  JSValue e = error::Ok();
  {
    Parser parser(u"{a: 1, \"b\": 123, a: \"c\"}");
    AST* ast = parser.ParseObjectLiteral();
    EnterGlobalCode(e, ast);
    JSValue val = EvalObject(e, ast);
    EXPECT_EQ(JSValue::OBJ_OBJECT, val.type());
    EXPECT_EQ(2, js_object::AllEnumerableProperties(val).size());
  }
}

TEST(TestEvalExpr, Array) {
  JSValue e = error::Ok();
  EnterGlobalCode(e, new ProgramOrFunctionBody(AST::AST_PROGRAM, false));
  {
    std::vector<std::pair<std::u16string, size_t>> sources = {
      {u"[, 1, 'abc', 123.4,]", 4}, {u"[]", 0}
    };
    for (auto pair : sources) {
      Parser parser(pair.first);
      AST* ast = parser.ParseArrayLiteral();
      JSValue val = EvalArray(e, ast);
      EXPECT_EQ(JSValue::OBJ_ARRAY, val.type());
      JSValue arr = val;
      EXPECT_EQ(pair.second, number::data(Get(e, arr, string::Length())));
    }
    
  }
}
