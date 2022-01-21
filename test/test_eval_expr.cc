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

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestEvalExpr, Number) {
  Error* e = Error::Ok();
  {
    std::vector<std::pair<string, double>> sources = {
      {u"0", 0}, {u"101", 101}, {u"0.01", 0.01}, {u"12.05", 12.05},
      {u".8", 0.8} ,{u"0xAbC09", 0xAbC09},
    };

    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      Number* num = EvalNumber(ast);
      EXPECT_EQ(pair.second, num->data());
    }
  }
}

TEST(TestEvalExpr, Arithmetic) {
  Init();
  Error* e = Error::Ok();
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
      JSValue* val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(JSValue::JS_NUMBER, val->type());
      Number* num = static_cast<Number*>(val);
      EXPECT_EQ(pair.second, num->data());
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
      JSValue* val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(JSValue::JS_NUMBER, val->type());
      Number* num = static_cast<Number*>(val);
      EXPECT_EQ(true, isnan(num->data()));
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
      JSValue* val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(JSValue::JS_NUMBER, val->type());
      Number* num = static_cast<Number*>(val);
      EXPECT_EQ(true, isinf(num->data()));
      EXPECT_EQ(pair.second, signbit(num->data()));
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
      JSValue* val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(JSValue::JS_NUMBER, val->type());
      Number* num = static_cast<Number*>(val);
      EXPECT_EQ(0, num->data());
      EXPECT_EQ(pair.second, signbit(num->data()));
    }
  }
}

TEST(TestEvalExpr, String) {
  Error* e = Error::Ok();
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
      String* str = EvalString(ast);
      EXPECT_EQ(pair.second, str->data());
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
      JSValue* val = EvalBinaryExpression(e, ast);
      EXPECT_EQ(JSValue::JS_STRING, val->type());
      String* str = static_cast<String*>(val);
      EXPECT_EQ(pair.second, str->data());
    }
  }
}

TEST(TestEvalExpr, Identifier) {
  Init();
  Error* e = Error::Ok();
  {
    Parser parser(u"a");
    AST* ast = parser.ParsePrimaryExpression();
    EnterGlobalCode(e, ast);
    Reference* ref = EvalIdentifier(ast);
    EXPECT_EQ(nullptr, GetValue(e, ref));
  }
}

TEST(TestEvalExpr, SimpleAssign) {
  Init();
  Error* e = Error::Ok();
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
  Init();
  Error* e = Error::Ok();
  {
    Parser parser(u"function (b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    EnterGlobalCode(e, ast);
    JSValue* val = EvalFunction(e, ast);
    EXPECT_EQ(JSValue::JS_OBJECT, val->type());
    FunctionObject* func = static_cast<FunctionObject*>(val);
    EXPECT_EQ(1, func->FormalParameters().size());
  }

  {
    Parser parser(u"function a(b) { return b; }");
    AST* ast = parser.ParseFunction(false);
    JSValue* val = EvalFunction(e, ast);
    EXPECT_EQ(JSValue::JS_OBJECT, val->type());
    FunctionObject* func = static_cast<FunctionObject*>(val);
    EXPECT_EQ(1, func->FormalParameters().size());
  }
}

TEST(TestEvalExpr, Object) {
  Error* e = Error::Ok();
  {
    Parser parser(u"{a: 1, \"b\": 123, a: \"c\"}");
    AST* ast = parser.ParseObjectLiteral();
    EnterGlobalCode(e, ast);
    JSValue* val = EvalObject(e, ast);
    EXPECT_EQ(JSValue::JS_OBJECT, val->type());
    Object* obj = static_cast<Object*>(val);
    EXPECT_EQ(2, obj->AllEnumerableProperties().size());
  }
}

TEST(TestEvalExpr, Array) {
  Error* e = Error::Ok();
  EnterGlobalCode(e, new ProgramOrFunctionBody(AST::AST_PROGRAM, false));
  {
    std::vector<std::pair<std::u16string, size_t>> sources = {
      {u"[, 1, 'abc', 123.4,]", 4}, {u"[]", 0}
    };
    for (auto pair : sources) {
      Parser parser(pair.first);
      AST* ast = parser.ParseArrayLiteral();
      JSValue* val = EvalArray(e, ast);
      EXPECT_EQ(JSValue::JS_OBJECT, val->type());
      ArrayObject* arr = static_cast<ArrayObject*>(val);
      EXPECT_EQ(pair.second, static_cast<Number*>(arr->Get(e, u"length"))->data());
    }
    
  }
}
