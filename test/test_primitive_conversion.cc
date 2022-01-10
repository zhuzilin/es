#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/types/conversion.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestPrimitiveConversion, ToPrimitive) {
  JSValue* val;
  Error* e;
  val = ToPrimitive(Undefined::Instance(), u"", e);
  EXPECT_EQ(JSValue::JS_UNDEFINED, val->type());

  val = ToPrimitive(Null::Instance(), u"", e);
  EXPECT_EQ(JSValue::JS_NULL, val->type());

  for (auto inner : {true, false}) {
    val = ToPrimitive(Bool::Wrap(inner), u"", e);
    EXPECT_EQ(JSValue::JS_BOOL, val->type());
    auto outer = static_cast<Bool*>(val);
    EXPECT_EQ(inner, outer->data());
  }

  for (auto inner : {1.0, 2.2, 3.5}) {
    val = ToPrimitive(new Number(inner), u"", e);
    EXPECT_EQ(JSValue::JS_NUMBER, val->type());
    auto outer = static_cast<Number*>(val);
    EXPECT_EQ(inner, outer->data());
  }

  for (auto inner : {u"abc", u"\n", u"你好", u"😎"}) {
    val = ToPrimitive(new String(inner), u"", e);
    EXPECT_EQ(JSValue::JS_STRING, val->type());
    auto outer = static_cast<String*>(val);
    EXPECT_EQ(inner, outer->data());
  }
}

TEST(TestPrimitiveConversion, ToBoolean) {
  Bool* b;
  Error* e;
  b = ToBoolean(Undefined::Instance());
  EXPECT_EQ(false, b->data());

  b = ToBoolean(Null::Instance());
  EXPECT_EQ(false, b->data());

  // Number
  for (auto num : {0.0, -0.0, Number::nan}) {
    b = ToBoolean(new Number(num));
    EXPECT_EQ(false, b->data());
  }
  b = ToBoolean(Number::NaN());
  EXPECT_EQ(false, b->data());

  for (auto num : {1.0, -1.0}) {
    b = ToBoolean(new Number(num));
    EXPECT_EQ(true, b->data());
  }

  // String
  b = ToBoolean(new String(u""));
  EXPECT_EQ(false, b->data());

  b = ToBoolean(new String(u"abc"));
  EXPECT_EQ(true, b->data());
}

TEST(TestPrimitiveConversion, ToNumber) {
  Number* num;
  Error* e;
  // Undefined
  num = ToNumber(Undefined::Instance(), e);
  EXPECT_EQ(Number::nan, num->data());

  // Null
  num = ToNumber(Null::Instance(), e);
  EXPECT_EQ(0, num->data());

  // Bool
  num = ToNumber(Bool::True(), e);
  EXPECT_EQ(1, num->data());
  num = ToNumber(Bool::False(), e);
  EXPECT_EQ(0, num->data());

  // String
  {
    // valid
    std::vector<std::pair<string, double>> vals = {
      {u" 0 ", 0}, {u" -42  ", -42}, {u"1.98", 1.98}, {u"4.e5 \t", 4.e5},
      {u"\n0xAB ", 0xAB}
    };
    for (auto pair : vals) {
      num = ToNumber(new String(pair.first), e);
      EXPECT_EQ(pair.second, num->data());
    }
  }
  // NaN
  {
    vec_string vals = {
      u"", u"+", u"+0xAB", u"0x", u"3e", u"\n+ 10",
    };
    for (auto val : vals) {
      num = ToNumber(new String(val), e);
      EXPECT_EQ(Number::nan, num->data());
    }
  }
  // Infinity
  {
    vec_string vals = {
      u"  Infinity\n ", u"\t +Infinity", u"-Infinity",
    };
    for (auto val : vals) {
      num = ToNumber(new String(val), e);
      EXPECT_EQ(true, num->IsInfinity());
    }
  }

}
