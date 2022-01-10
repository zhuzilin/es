#ifndef ES_TYPES_CONVERSION_H
#define ES_TYPES_CONVERSION_H

#include <math.h>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/types/builtin/number_object.h>
#include <es/error.h>

namespace es {

JSValue* ToPrimitive(JSValue* input, std::u16string_view preferred_type, Error* e) {
  assert(input->IsLanguageType());
  if (input->IsPrimitive()) {
    return input;
  }
  JSObject* obj = static_cast<JSObject*>(input);
  return obj->DefaultValue(preferred_type, e);
}

Bool* ToBoolean(JSValue* input) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
    case JSValue::JS_NULL:
      return Bool::False();
    case JSValue::JS_BOOL:
      return Bool::False();
    case JSValue::JS_NUMBER: {
      Number* num = static_cast<Number*>(input);
      if (num->data() == 0 || num->data() == Number::nan) {
        return Bool::False();
      }
      return Bool::True();
    }
    case JSValue::JS_STRING: {
      String* str = static_cast<String*>(input);
      return Bool::Wrap(str->data() != u"");
    } 
    case JSValue::JS_OBJECT:
      return Bool::True();
    default:
      assert(false);
  }
}

Number* StringToNumber(String* str) {
  auto source = str->data();
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
  if (start == end) {
    goto error;
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
    return new Number(val);
  }

  if (start == end)
    goto error;

  if (source.substr(start, end - start) == u"Infinity") {
    return new Number(0, positive ? 1 : -1);
  }

  while (start < end) {
    char16_t c = source[start];
    if (!character::IsDecimalDigit(c))
      break;
    val = val * 10 + character::Digit(c);
    start++;
  }
  if (start == end)
    return new Number(positive ? val : -val);
  
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
      return new Number(positive ? val : -val);
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
    if (start == end) {
      val = val * pow(10, exp);
      return new Number(positive ? val : -val);
    }
  }
error:
  return Number::NaN();
}

Number* ToNumber(JSValue* input, Error* e) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
      return Number::NaN();
    case JSValue::JS_NULL:
      return Number::Zero();
    case JSValue::JS_BOOL:
      return new Number(static_cast<Bool*>(input)->data() ? 1 : 0);
    case JSValue::JS_NUMBER:
      return static_cast<Number*>(input);
    case JSValue::JS_STRING:
      return StringToNumber(static_cast<String*>(input));
    // case JSValue::JS_OBJECT:
    //   JSValue* prim_value = ToPrimitive(input, u"Number", e);
    //   return ToNumber(prim_value, e);
    default:
      assert(false);
  }
}

Number* ToInteger(JSValue* input, Error* e) {
  Number* num = ToNumber(input, e);
  if (num->IsNaN()) {
    return Number::Zero();
  }
  if (num->IsInfinity() || num->data() == 0) {
    return num;
  }
  double data = num->data();
  return new Number(data > 0 ? floor(abs(data)) : -(floor(abs(-data))));
}

Number* ToInt32(JSValue* input, Error* e) {
  Number* num = ToNumber(input, e);
  if (num->IsNaN() || num->IsInfinity() || num->data() == 0) {
    return Number::Zero();
  }
  double pos_int = ToInteger(num, e)->data();
  double int32_bit = fmod(pos_int, pow(2, 32));
  if (int32_bit < 0)
    int32_bit += pow(2, 32);

  if (int32_bit > pow(2, 31)) {
    return new Number(int32_bit - pow(2, 32));
  } else {
    return new Number(int32_bit);
  }
}

Number* ToUint(JSValue* input, char bits, Error* e) {
  Number* num = ToNumber(input, e);
  if (num->IsNaN() || num->IsInfinity() || num->data() == 0) {
    return Number::Zero();
  }
  double pos_int = ToInteger(num, e)->data();
  double int_bit = fmod(pos_int, pow(2, bits));
  if (int_bit < 0)
    int_bit += pow(2, bits);
  return new Number(int_bit);
}

Number* ToUint32(JSValue* input, Error* e) {
  return ToUint(input, 32, e);
}

Number* ToUint16(JSValue* input, Error* e) {
  return ToUint(input, 16, e);
}

String* NumberToString(Number* num) {
  if (num->IsNaN())
    return String::NaN();
  if (num->IsInfinity())
    return String::Infinity();
  if (num->data() == 0)
    return String::Zero();
  assert(false);
}

String* ToString(JSValue* input, Error* e) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
      return String::Undefined();
    case JSValue::JS_NULL:
      return String::Null();
    case JSValue::JS_BOOL:
      return static_cast<Bool*>(input)->data() ? String::True() : String::False();
    case JSValue::JS_NUMBER:
      return NumberToString(static_cast<Number*>(input));
    case JSValue::JS_STRING:
      return static_cast<String*>(input);
    // case JSValue::JS_OBJECT:
    //   JSValue* prim_value = ToPrimitive(input, u"String", e);
    //   return ToString(prim_value, e);
    default:
      assert(false);
  }
}

JSObject* ToObject(JSValue* input, Error* e) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
    case JSValue::JS_NULL:
      e = Error::TypeError();
      return nullptr;
    case JSValue::JS_BOOL:
      assert(false);
    case JSValue::JS_NUMBER:
      return new NumberObject(input);
    case JSValue::JS_STRING:
      assert(false);
    case JSValue::JS_OBJECT:
      return static_cast<JSObject*>(input);
    default:
      assert(false);
}

}  // namespace es

}  // namespace es

#endif  // ES_TYPES_CONVERSION_H