#include <math.h>

#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/types/conversion.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/utils/helper.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestPrimitiveConversion, ToPrimitive) {
  JSValue* val;
  Error* e = Error::Ok();
  val = ToPrimitive(e, Undefined::Instance(), u"");
  EXPECT_EQ(JSValue::JS_UNDEFINED, val->type());

  val = ToPrimitive(e, Null::Instance(), u"");
  EXPECT_EQ(JSValue::JS_NULL, val->type());

  for (auto inner : {true, false}) {
    val = ToPrimitive(e, Bool::Wrap(inner), u"");
    EXPECT_EQ(JSValue::JS_BOOL, val->type());
    auto outer = static_cast<Bool*>(val);
    EXPECT_EQ(inner, outer->data());
  }

  for (auto inner : {1.0, 2.2, 3.5}) {
    val = ToPrimitive(e, new Number(inner), u"");
    EXPECT_EQ(JSValue::JS_NUMBER, val->type());
    auto outer = static_cast<Number*>(val);
    EXPECT_EQ(inner, outer->data());
  }

  for (auto inner : {u"abc", u"\n", u"你好", u"😎"}) {
    val = ToPrimitive(e, new String(inner), u"");
    EXPECT_EQ(JSValue::JS_STRING, val->type());
    auto outer = static_cast<String*>(val);
    EXPECT_EQ(inner, outer->data());
  }
}

TEST(TestPrimitiveConversion, ToBoolean) {
  bool b;
  Error* e = Error::Ok();
  b = ToBoolean(Undefined::Instance());
  EXPECT_EQ(false, b);

  b = ToBoolean(Null::Instance());
  EXPECT_EQ(false, b);

  // Number
  for (double num : {0.0, -0.0, nan("")}) {
    b = ToBoolean(new Number(num));
    EXPECT_EQ(false, b);
  }
  b = ToBoolean(Number::NaN());
  EXPECT_EQ(false, b);

  for (auto num : {1.0, -1.0}) {
    b = ToBoolean(new Number(num));
    EXPECT_EQ(true, b);
  }

  // String
  b = ToBoolean(new String(u""));
  EXPECT_EQ(false, b);

  b = ToBoolean(new String(u"abc"));
  EXPECT_EQ(true, b);
}

TEST(TestPrimitiveConversion, ToNumber) {
  double num;
  Error* e = Error::Ok();
  // Undefined
  num = ToNumber(e, Undefined::Instance());
  EXPECT_EQ(true, isnan(num));

  // Null
  num = ToNumber(e, Null::Instance());
  EXPECT_EQ(0, num);

  // Bool
  num = ToNumber(e, Bool::True());
  EXPECT_EQ(1, num);
  num = ToNumber(e, Bool::False());
  EXPECT_EQ(0, num);

  // String
  {
    // valid
    std::vector<std::pair<string, double>> vals = {
      {u" 0 ", 0}, {u" -42  ", -42}, {u"1.98", 1.98}, {u"4.e5 \t", 4.e5},
      {u"\n0xAB ", 0xAB}
    };
    for (auto pair : vals) {
      num = ToNumber(e, new String(pair.first));
      EXPECT_EQ(pair.second, num);
    }
  }
  // NaN
  {
    vec_string vals = {
      u"", u"+", u"+0xAB", u"0x", u"3e", u"\n+ 10",
    };
    for (auto val : vals) {
      num = ToNumber(e, new String(val));
      EXPECT_EQ(true, isnan(num));
    }
  }
  // Infinity
  {
    vec_string vals = {
      u"  Infinity\n ", u"\t +Infinity", u"-Infinity",
    };
    for (auto val : vals) {
      num = ToNumber(e, new String(val));
      EXPECT_EQ(true, isinf(num));
    }
  }

}

TEST(TestPrimitiveConversion, ToInteger) {
  double num;
  Error* e = Error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4},
    };
    for (auto pair : vals) {
      num = ToInteger(e, new Number(pair.first));
      EXPECT_EQ(pair.second, num);
    }
  }

  num = ToInteger(e, Number::NaN());
  EXPECT_EQ(0, num);

  num = ToInteger(e, Number::PositiveInfinity());
  EXPECT_EQ(true, isinf(num) && !signbit(num));
}

TEST(TestPrimitiveConversion, ToInt32) {
  double num, num1;
  Error* e = Error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4}, {pow(2, 31) + 2, 2 - pow(2, 31)}, {pow(2, 33), 0}
    };
    for (auto pair : vals) {
      num = ToInt32(e, new Number(pair.first));
      EXPECT_EQ(pair.second, num);
      // idempotent
      num1 = ToInt32(e, new Number(num));
      EXPECT_EQ(num, num1);
    }
  }

  num = ToInt32(e, Number::NaN());
  EXPECT_EQ(0, num);

  num = ToInt32(e, Number::PositiveInfinity());
  EXPECT_EQ(0, num);
}

TEST(TestPrimitiveConversion, ToUint32) {
  double num, num1;
  Error* e = Error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4 + pow(2, 32)}, {pow(2, 31) + 2, pow(2, 31) + 2}, {pow(2, 33), 0}
    };
    for (auto pair : vals) {
      num = ToUint32(e, new Number(pair.first));
      EXPECT_EQ(pair.second, num);
      // idempotent
      num1 = ToUint32(e, new Number(num));
      EXPECT_EQ(num, num1);
    }
  }

  num = ToUint32(e, Number::NaN());
  EXPECT_EQ(0, num);

  num = ToUint32(e, Number::PositiveInfinity());
  EXPECT_EQ(0, num);
}
