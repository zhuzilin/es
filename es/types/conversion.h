#ifndef ES_TYPES_CONVERSION_H
#define ES_TYPES_CONVERSION_H

#include <math.h>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/string_object.h>
#include <es/error.h>

namespace es {

Handle<JSValue> ToPrimitive(Handle<Error>& e, Handle<JSValue> input, std::u16string preferred_type) {
  ASSERT(input.val()->IsLanguageType());
  if (input.val()->IsPrimitive()) {
    return input;
  }
  Handle<JSObject> obj = static_cast<Handle<JSObject>>(input);
  return DefaultValue(e, obj, preferred_type);
}

bool ToBoolean(Handle<JSValue> input) {
  ASSERT(input.val()->IsLanguageType());
  switch (input.val()->type()) {
    case Type::JS_UNDEFINED:
    case Type::JS_NULL:
      return false;
    case Type::JS_BOOL:
      return static_cast<Handle<Bool>>(input).val()->data();
    case Type::JS_NUMBER: {
      Handle<Number> num = static_cast<Handle<Number>>(input);
      if (num.val()->data() == 0.0 || num.val()->data() == -0.0 || num.val()->IsNaN()) {
        return false;
      }
      return true;
    }
    case Type::JS_LONG_STRING:
    case Type::JS_STRING: {
      Handle<String> str = static_cast<Handle<String>>(input);
      return !StringEqual(str, String::Empty());
    }
    default:
      if (input.val()->IsObject())
        return true;
      assert(false);
  }
}

// 9.3.1 ToNumber Applied to the String Type
double StringToNumber(std::u16string source) {
  size_t start = 0;
  size_t end = source.size();
  bool positive = true;
  double val = 0;
  while (start < end) {
    char16_t c = source[start];
    if (!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
      break;
    start++;
  }
  while (start < end) {
    char16_t c = source[end - 1];
    if (!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
      break;
    end--;
  }
  // The MV of [empty] is 0
  if (start == end) {
    return 0;
  } else if (source[start] == u'-') {
    positive = false;
    start++;
  } else if (source[start] == u'+') {
    start++;
  } else if (end - start > 2 && source[start] == u'0' &&
             (source[start + 1] == u'x' || source[start + 1] == u'X')){
    // 0xABCD...
    start += 2;
    while (start < end) {
      char16_t c = source[start];
      if (!character::IsHexDigit(c))
        goto error;
      val = val * 16 + character::Digit(c);
      start++;
    }
    return val;
  }

  if (start == end)
    goto error;

  if (source.substr(start, end - start) == u"Infinity") {
    return positive ? Number::Infinity().val()->data() : Number::NegativeInfinity().val()->data();
  }

  while (start < end) {
    char16_t c = source[start];
    if (!character::IsDecimalDigit(c))
      break;
    val = val * 10 + character::Digit(c);
    start++;
  }
  if (start == end)
    return positive ? val : -val;
  
  if (source[start] == u'.') {
    start++;
    double frac = 0.1;
    while (start < end) {
      char16_t c = source[start];
      if (!character::IsDecimalDigit(c)) {
        break;
      }
      val += frac * character::Digit(c);
      frac /= 10;
      start++;
    }
    if (start == end)
      return positive ? val : -val;
  }

  if (source[start] == u'e' || source[start] == u'E') {
    start++;
    double exp = 0;
    bool exp_positive = true;
    if (start == end)
      goto error;
    if (source[start] == u'-') {
      exp_positive = false;
      start++;
    } else if (source[start] == u'-') {
      start++;
    }
    if (start == end)
      goto error;

    while (start < end) {
      char16_t c = source[start];
      if (!character::IsDecimalDigit(c)) {
        break;
      }
      exp = exp * 10 + character::Digit(c);
      start++;
    }
    if (!exp_positive)
      exp = -exp;
    if (start == end) {
      val = val * pow(10, exp);
      return positive ? val : -val;
    }
  }
error:
  return nan("");
}

constexpr char16_t max_uint32_str[11] = u"4294967295";

bool ToArrayIndex(const char16_t* str, size_t n, double& res) {
  if (n > 10 || n == 0 || str[0] < u'0' || str[0] > u'9')
    return false;
  if (n != 1 && str[0] == u'0')
    return false;

  uint32_t val = 0;
  if (unlikely(n == 10)) {
    bool equal_max = true;
    for (uint8_t i = 0; i < n; ++i) {
      if (!(u'0' <= str[i] && str[i] <= u'9')) {
        return false;
      }
      if (equal_max && str[i] > max_uint32_str[i]) {
        return false;
      }
      equal_max = equal_max && str[i] == max_uint32_str[i];
      val = 10 * val + (str[i] - u'0');
    }
  } else {
    for (uint8_t i = 0; i < n; ++i) {
      if (!(u'0' <= str[i] && str[i] <= u'9')) {
        return false;
      }
      val = 10 * val + (str[i] - u'0');
    }
  }
  res = val;
  return true;
}

std::u16string ArrayIndexToString(uint32_t index) {
  if (index == 0) return u"0";
  std::u16string s = u"";
  while (index > 0) {
    s = static_cast<char16_t>(u'0' + index % 10) + s;
    index /= 10;
  }
  return s;
}

double StringToNumber(Handle<String> str) {
  if (str.val()->IsArrayIndex())
    return str.val()->Index();
  return StringToNumber(str.val()->data());
}

double ToNumber(Handle<Error>& e, Handle<JSValue> input) {
  ASSERT(input.val()->IsLanguageType());
  switch (input.val()->type()) {
    case Type::JS_UNDEFINED:
      return nan("");
    case Type::JS_NULL:
      return 0.0;
    case Type::JS_BOOL:
      return static_cast<Handle<Bool>>(input).val()->data() ? 1.0 : 0.0;
    case Type::JS_NUMBER:
      return static_cast<Handle<Number>>(input).val()->data();
    case Type::JS_LONG_STRING:
    case Type::JS_STRING:
      return StringToNumber(static_cast<Handle<String>>(input));
    default:
      if (input.val()->IsObject()) {
        Handle<JSValue> prim_value = ToPrimitive(e, input, u"Number");
        if (unlikely(!e.val()->IsOk())) return 0.0;
        return ToNumber(e, prim_value);
      }
      assert(false);
  }
}

double ToInteger(Handle<Error>& e, Handle<JSValue> input) {
  double num = ToNumber(e, input);
  if (unlikely(!e.val()->IsOk())) return 0.0;
  if (isnan(num)) {
    return 0.0;
  }
  if (isinf(num) || num == 0) {
    return num;
  }
  return num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
}

double ToInt32(Handle<Error>& e, Handle<JSValue> input) {
  double num = ToNumber(e, input);
  if (unlikely(!e.val()->IsOk()))
    return 0;
  if (isnan(num) || isinf(num) || num == 0) {
    return 0.0;
  }
  double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
  double int32_bit = fmod(pos_int, pow(2, 32));
  if (int32_bit < 0)
    int32_bit += pow(2, 32);

  if (int32_bit > pow(2, 31)) {
    return int32_bit - pow(2, 32);
  } else {
    return int32_bit;
  }
}

double ToUint(Handle<Error>& e, Handle<JSValue> input, char bits) {
  double num = ToNumber(e, input);
  if (unlikely(!e.val()->IsOk())) return 0.0;
  if (isnan(num) || isinf(num) || num == 0) {
    return 0.0;
  }
  double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
  double int_bit = fmod(pos_int, pow(2, bits));
  if (int_bit < 0)
    int_bit += pow(2, bits);
  return int_bit;
}

double ToUint32(Handle<Error>& e, Handle<JSValue> input) {
  return ToUint(e, input, 32);
}

double ToUint16(Handle<Error>& e, Handle<JSValue> input) {
  return ToUint(e, input, 16);
}

std::u16string NumberToU16String(double m) {
  if (m == 0)
    return u"0";
  if (isnan(m))
    return u"NaN";
  if (isinf(m))
    return signbit(m) ? u"-Infinity" : u"Infinity";
  // TODO(zhuzilin) Figure out how to solve the large number error.
  std::u16string sign = u"";
  if (m < 0) {
    m = -m;
    sign = u"-";
  }
  // the fraction digits, e.g. 1.23's frac_digit = 2, 4200's = -2
  int frac_digit = 0;
  // the total digits, e.g. 1.23's k = 3, 4200's k = 2
  int k = 0;
  // n - k = -frac_digit
  int n = 0;
  double tmp;
  while (modf(m, &tmp) != 0) {
    frac_digit++;
    m *= 10;
  }
  while (m != 0 && fmod(m, 10) < 1e-6) {
    frac_digit--;
    m /= 10;
  }
  double s = m;
  while (m > 0.5) {
    k++;
    m /= 10;
    modf(m, &tmp);
    m = tmp;
  }
  n = k - frac_digit;
  std::u16string res = u"";
  if (k <= n && n <= 21) {
    while (s > 0.5) {
      res += u'0' + int(fmod(s, 10));
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    res += std::u16string(n - k, u'0');
    return sign + res;
  }
  if (0 < n && n <= 21) {
    for (int i = 0; i < k; i++) {
      res += u'0' + int(fmod(s, 10));
      if (i + 1 == k - n) {
        res += u'.';
      }
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    return sign + res;
  }
  if (-6 < n && n <= 0) {
    for (int i = 0; i < k; i++) {
      res += u'0' + int(fmod(s, 10));
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    res = u"0." + std::u16string(-n, u'0') + res;
    return sign + res;
  }
  if (k == 1) {
    res += u'0' + int(s);
    res += u"e";
    if (n - 1 > 0) {
      res += u"+" + NumberToU16String(n - 1);
    } else {
      res += u"-" + NumberToU16String(1 - n);
    }
    return sign + res;
  }
  for (int i = 0; i < k; i++) {
    res += u'0' + int(fmod(s, 10));
    if (i + 1 == k - 1) {
      res += u'.';
    }
    s /= 10;
    modf(s, &tmp);
    s = tmp;
  }
  res += u"e";
  if (n - 1 > 0) {
    res += u"+" + NumberToU16String(n - 1);
  } else {
    res += u"-" + NumberToU16String(1 - n);
  }
  return sign + res;
}

std::string NumberToStdString(double m) {
  if (m == 0)
    return "0";
  if (isnan(m))
    return "NaN";
  if (isinf(m))
    return signbit(m) ? "-Infinity" : "Infinity";
  // TODO(zhuzilin) Figure out how to solve the large number error.
  std::string sign = "";
  if (m < 0) {
    m = -m;
    sign = "-";
  }
  // the fraction digits, e.g. 1.23's frac_digit = 2, 4200's = -2
  int frac_digit = 0;
  // the total digits, e.g. 1.23's k = 3, 4200's k = 2
  int k = 0;
  // n - k = -frac_digit
  int n = 0;
  double tmp;
  while (modf(m, &tmp) != 0) {
    frac_digit++;
    m *= 10;
  }
  while (m != 0 && fmod(m, 10) < 1e-6) {
    frac_digit--;
    m /= 10;
  }
  double s = m;
  while (m > 0.5) {
    k++;
    m /= 10;
    modf(m, &tmp);
    m = tmp;
  }
  n = k - frac_digit;
  std::string res = "";
  if (k <= n && n <= 21) {
    while (s > 0.5) {
      res += '0' + int(fmod(s, 10));
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    res += std::string(n - k, '0');
    return sign + res;
  }
  if (0 < n && n <= 21) {
    for (int i = 0; i < k; i++) {
      res += '0' + int(fmod(s, 10));
      if (i + 1 == k - n) {
        res += '.';
      }
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    return sign + res;
  }
  if (-6 < n && n <= 0) {
    for (int i = 0; i < k; i++) {
      res += '0' + int(fmod(s, 10));
      s /= 10;
      modf(s, &tmp);
      s = tmp;
    }
    reverse(res.begin(), res.end());
    res = "0." + std::string(-n, '0') + res;
    return sign + res;
  }
  if (k == 1) {
    res += '0' + int(s);
    res += "e";
    if (n - 1 > 0) {
      res += "+" + NumberToStdString(n - 1);
    } else {
      res += "-" + NumberToStdString(1 - n);
    }
    return sign + res;
  }
  for (int i = 0; i < k; i++) {
    res += '0' + int(fmod(s, 10));
    if (i + 1 == k - 1) {
      res += '.';
    }
    s /= 10;
    modf(s, &tmp);
    s = tmp;
  }
  res += "e";
  if (n - 1 > 0) {
    res += "+" + NumberToStdString(n - 1);
  } else {
    res += "-" + NumberToStdString(1 - n);
  }
  return sign + res;
}

Handle<String> NumberToString(double m) {
  if (isnan(m))
    return String::NaN();
  if (isinf(m))
    return signbit(m) ? String::NegativeInfinity() : String::Infinity();
  if (static_cast<double>(static_cast<uint32_t>(m)) == m)
    return String::New(static_cast<uint32_t>(m));
  return String::New(NumberToU16String(m));
}

Handle<String> NumberToStringConst(double m) {
#ifdef PARSER_TEST
  std::cout << "NumberToStringConst " << m << std::endl;
#endif
  if (isnan(m))
    return String::NaN();
  if (isinf(m))
    return signbit(m) ? String::NegativeInfinity() : String::Infinity();
  if (static_cast<double>(static_cast<uint32_t>(m)) == m)
    return String::New(static_cast<uint32_t>(m));
  return String::New<GCFlag::CONST>(NumberToU16String(m));
}


Handle<String> NumberToString(Handle<Number> num) {
  return NumberToString(num.val()->data());
}

Handle<String> ToString(Handle<Error>& e, Handle<JSValue> input) {
  ASSERT(input.val()->IsLanguageType());
  switch (input.val()->type()) {
    case Type::JS_UNDEFINED:
      return String::undefined();
    case Type::JS_NULL:
      return String::Null();
    case Type::JS_BOOL:
      return static_cast<Handle<Bool>>(input).val()->data() ? String::True() : String::False();
    case Type::JS_NUMBER:
      return NumberToString(static_cast<Handle<Number>>(input));
    case Type::JS_LONG_STRING:
    case Type::JS_STRING:
      return static_cast<Handle<String>>(input);
    default:
      if (input.val()->IsObject()) {
        Handle<JSValue> prim_value = ToPrimitive(e, input, u"String");
        if (unlikely(!e.val()->IsOk())) return String::Empty();
        return ToString(e, prim_value);
      }
      assert(false);
  }
}

std::u16string ToU16String(Handle<Error>& e, Handle<JSValue> input) {
  ASSERT(input.val()->IsLanguageType());
  switch (input.val()->type()) {
    case Type::JS_NUMBER:
      return NumberToU16String(static_cast<Handle<Number>>(input).val()->data());
    default:
      return ToString(e, input).val()->data();
  }
}

Handle<JSObject> ToObject(Handle<Error>& e, Handle<JSValue> input) {
  ASSERT(input.val()->IsLanguageType());
  switch (input.val()->type()) {
    case Type::JS_UNDEFINED:
    case Type::JS_NULL:
      e = Error::TypeError(u"Cannot convert undefined or null to object");
      return Handle<JSObject>();
    case Type::JS_BOOL:
      return BoolObject::New(input);
    case Type::JS_NUMBER:
      return NumberObject::New(input);
    case Type::JS_LONG_STRING:
    case Type::JS_STRING:
      return StringObject::New(input);
    default:
      if (input.val()->IsObject()) {
        return static_cast<Handle<JSObject>>(input);
      }
      assert(false);
}

}  // namespace es

}  // namespace es

#endif  // ES_TYPES_CONVERSION_H