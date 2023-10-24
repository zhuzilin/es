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
#include <es/gc/heap.h>
#include <es/impl.h>
#include <es/eval.h>
#include <es/enter_code.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestPrimitiveConversion, ToPrimitive) {
  JSValue val;
  JSValue e = error::Ok();
  val = ToPrimitive(e, undefined::New(), u"");
  EXPECT_EQ(Type::JS_UNDEFINED, val.type());

  val = ToPrimitive(e, null::New(), u"");
  EXPECT_EQ(Type::JS_NULL, val.type());

  for (auto inner : {true, false}) {
    val = ToPrimitive(e, boolean::New(inner), u"");
    EXPECT_EQ(Type::JS_BOOL, val.type());
    EXPECT_EQ(inner, boolean::data(val));
  }

  for (auto inner : {1.0, 2.2, 3.5}) {
    val = ToPrimitive(e, number::New(inner), u"");
    EXPECT_EQ(Type::JS_NUMBER, val.type());
    EXPECT_EQ(inner, number::data(val));
  }

  for (auto inner : {u"abc", u"\n", u"你好", u"😎"}) {
    val = ToPrimitive(e, string::New(inner), u"");
    EXPECT_EQ(Type::JS_STRING, val.type());
    EXPECT_EQ(inner, number::data(val));
  }
}

TEST(TestPrimitiveConversion, ToBoolean) {
  bool b;
  JSValue e = error::Ok();
  b = ToBoolean(undefined::New());
  EXPECT_EQ(false, b);

  b = ToBoolean(null::New());
  EXPECT_EQ(false, b);

  // Number
  for (double num : {0.0, -0.0, nan("")}) {
    b = ToBoolean(number::New(num));
    EXPECT_EQ(false, b);
  }
  b = ToBoolean(number::NaN());
  EXPECT_EQ(false, b);

  for (auto num : {1.0, -1.0}) {
    b = ToBoolean(number::New(num));
    EXPECT_EQ(true, b);
  }

  // String
  b = ToBoolean(string::New(u""));
  EXPECT_EQ(false, b);

  b = ToBoolean(string::New(u"abc"));
  EXPECT_EQ(true, b);
}

TEST(TestPrimitiveConversion, ToNumber) {
  double num;
  JSValue e = error::Ok();
  // Undefined
  num = ToNumber(e, undefined::New());
  EXPECT_EQ(true, isnan(num));

  // Null
  num = ToNumber(e, null::New());
  EXPECT_EQ(0, num);

  // Bool
  num = ToNumber(e, boolean::True());
  EXPECT_EQ(1, num);
  num = ToNumber(e, boolean::False());
  EXPECT_EQ(0, num);

  // String
  {
    // valid
    std::vector<std::pair<string, double>> vals = {
      {u" 0 ", 0}, {u" -42  ", -42}, {u"1.98", 1.98}, {u"4.e5 \t", 4.e5},
      {u"\n0xAB ", 0xAB}, {u"", 0}
    };
    for (auto pair : vals) {
      num = ToNumber(e, string::New(pair.first));
      EXPECT_EQ(pair.second, num);
    }
  }
  // NaN
  {
    vec_string vals = {
      u"+", u"+0xAB", u"0x", u"3e", u"\n+ 10",
    };
    for (auto val : vals) {
      num = ToNumber(e, string::New(val));
      EXPECT_EQ(true, isnan(num));
    }
  }
  // Infinity
  {
    vec_string vals = {
      u"  Infinity\n ", u"\t +Infinity", u"-Infinity",
    };
    for (auto val : vals) {
      num = ToNumber(e, string::New(val));
      EXPECT_EQ(true, isinf(num));
    }
  }

}

TEST(TestPrimitiveConversion, ToInteger) {
  double num;
  JSValue e = error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4},
    };
    for (auto pair : vals) {
      num = ToInteger(e, number::New(pair.first));
      EXPECT_EQ(pair.second, num);
    }
  }

  num = ToInteger(e, number::NaN());
  EXPECT_EQ(0, num);

  num = ToInteger(e, number::Infinity());
  EXPECT_EQ(true, isinf(num) && !signbit(num));
}

TEST(TestPrimitiveConversion, ToInt32) {
  double num, num1;
  JSValue e = error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4}, {pow(2, 31) + 2, 2 - pow(2, 31)}, {pow(2, 33), 0}
    };
    for (auto pair : vals) {
      num = ToInt32(e, number::New(pair.first));
      EXPECT_EQ(pair.second, num);
      // idempotent
      num1 = ToInt32(e, number::New(num));
      EXPECT_EQ(num, num1);
    }
  }

  num = ToInt32(e, number::NaN());
  EXPECT_EQ(0, num);

  num = ToInt32(e, number::Infinity());
  EXPECT_EQ(0, num);
}

TEST(TestPrimitiveConversion, ToUint32) {
  double num, num1;
  JSValue e = error::Ok();
  {
    std::vector<std::pair<double, double>> vals = {
      {4.2, 4}, {-4.2, -4 + pow(2, 32)}, {pow(2, 31) + 2, pow(2, 31) + 2}, {pow(2, 33), 0}
    };
    for (auto pair : vals) {
      num = ToUint32(e, number::New(pair.first));
      EXPECT_EQ(pair.second, num);
      // idempotent
      num1 = ToUint32(e, number::New(num));
      EXPECT_EQ(num, num1);
    }
  }

  num = ToUint32(e, number::NaN());
  EXPECT_EQ(0, num);

  num = ToUint32(e, number::Infinity());
  EXPECT_EQ(0, num);
}

TEST(TestPrimitiveConversion, ToString) {
  Handle<String> str;
  JSValue e = error::Ok();
  // String
  {
    // valid
    std::vector<std::pair<double, string>> vals = {
      {0, u"0"}, {-42, u"-42"}, {1200, u"1200"}, {1.230, u"1.23"},
      // TODO(zhuzilin) deal with this...
      // {0.0012, u"0.0011999999999999998"},
      // {3e50, u"3e+50"}
    };
    for (auto pair : vals) {
      str = ToString(e, number::New(pair.first));
      EXPECT_EQ(pair.second, str.val()->data());
    }
  }
}
